/*
 * probe_processing.c
 *
 *  Created on: May 29, 2026
 *      Author: neill-lambert
 */


#include <stdint.h>
#include <stdlib.h>
#include "math.h"
#include "arm_math.h"
#include <string.h>
#include "probe_processing.h"

#define FFT_SIZE        1024
#define HOP_SIZE        256   // 75% overlap
#define NUM_OSCILLATORS 48
#define SINE_TABLE_SIZE 1024
#define SAMPLE_RATE     48000.0f

// One analysis frame
typedef struct {
    float magnitude[FFT_SIZE/2];
    float phase[FFT_SIZE/2];
    float frequency[FFT_SIZE/2];  // instantaneous freq via phase diff
} AnalysisFrame;

// One additive oscillator
typedef struct {
    float frequency;      // Hz
    float amplitude;      // 0..1
    float target_freq;    // for interpolation between frames
    float target_amp;
    float phase;          // current phase 0..2pi
    uint8_t active;
} Oscillator;

// Spectral transformation parameters
typedef struct {
    float pitch_shift;    // semitones, 0 = no shift
    float freeze;         // 0 = live, 1 = fully frozen
    float smear;          // 0..1 spectral blur amount
    float scramble;       // 0..1 bin randomisation
} SpectralParams;

// Low Pass Gate state
typedef struct {
    float z1, z2;
    float resonance;
    float env_amount;     // how much envelope opens the gate
    float env;            // current envelope follower value
    float attack;
    float release;
} LowPassGate;

typedef struct {
    float envelope;
    float decay;
    uint8_t triggered;
} Excitation;

// The full context — one allocation, everything inside
struct ProbeContext {
    // Analysis
    arm_rfft_fast_instance_f32 fft_inst;
    float analysis_buf[FFT_SIZE];
    float fft_buf[FFT_SIZE];
    float window[FFT_SIZE];
    float prev_phase[FFT_SIZE/2];
    AnalysisFrame frame;

    // Hop accumulator
    float hop_buf[FFT_SIZE];
    uint32_t hop_counter;

    // Resynthesis
    Oscillator oscs[NUM_OSCILLATORS];
    float sine_table[SINE_TABLE_SIZE];

    // Spectral transforms
    SpectralParams spectral;
    float frozen_mag[FFT_SIZE/2];
    uint8_t have_freeze;

    // Excitation
    Excitation exc;

    // Output shaping
    LowPassGate lpg;
};

static void _init_internals(ProbeContext *ctx)
{
    // FFT
    arm_rfft_fast_init_f32(&ctx->fft_inst, FFT_SIZE);

    // Hanning window
    for (int i = 0; i < FFT_SIZE; i++)
        ctx->window[i] = 0.5f * (1.0f - cosf(2.0f * M_PI * i / (FFT_SIZE - 1)));

    // Sine table
    for (int i = 0; i < SINE_TABLE_SIZE; i++)
        ctx->sine_table[i] = sinf(2.0f * M_PI * i / SINE_TABLE_SIZE);

    // LPG defaults
    ctx->lpg.resonance = 0.3f;
    ctx->lpg.attack    = 0.01f;   // fast attack
    ctx->lpg.release   = 0.001f;  // slow release

    // Spectral defaults
    ctx->spectral.pitch_shift = 0.0f;
    ctx->spectral.freeze      = 0.0f;
    ctx->spectral.smear       = 0.0f;

    // Hop counter
    ctx->hop_counter = 0;
    ctx->have_freeze = 0;

    // Zero oscillators
    memset(ctx->oscs, 0, sizeof(ctx->oscs));

    // Zero analysis buffers
    memset(ctx->analysis_buf, 0, sizeof(ctx->analysis_buf));
    memset(ctx->prev_phase,   0, sizeof(ctx->prev_phase));
    memset(ctx->fft_buf,      0, sizeof(ctx->fft_buf));
    memset(ctx->hop_buf,      0, sizeof(ctx->hop_buf));
}

static ProbeContext probe_instance;

ProbeContext *probe_init(void)
{
    ProbeContext *ctx = &probe_instance;
    memset(ctx, 0, sizeof(*ctx));
    // init FFT, window, sine table etc
    _init_internals(ctx);
    return ctx;
}


void probe_free(ProbeContext *ctx)
{
    (void)ctx;  // nothing to free
}


static void _init_window(ProbeContext *ctx)
{
    for (int i = 0; i < FFT_SIZE; i++)
        ctx->window[i] = 0.5f * (1.0f - cosf(2.0f * M_PI * i / (FFT_SIZE - 1)));
}

static void _init_sine_table(ProbeContext *ctx)
{
    for (int i = 0; i < SINE_TABLE_SIZE; i++)
        ctx->sine_table[i] = sinf(2.0f * M_PI * i / SINE_TABLE_SIZE);
}

static void _analyse_frame(ProbeContext *ctx)
{
    float windowed[FFT_SIZE];
    arm_mult_f32(ctx->analysis_buf, ctx->window, windowed, FFT_SIZE);
    arm_rfft_fast_f32(&ctx->fft_inst, windowed, ctx->fft_buf, 0);

    float bin_freq  = SAMPLE_RATE / FFT_SIZE;
    float hop_angle = 2.0f * M_PI * HOP_SIZE / FFT_SIZE;

    for (int k = 0; k < FFT_SIZE/2; k++)
    {
        float re = ctx->fft_buf[2*k];
        float im = ctx->fft_buf[2*k + 1];

        ctx->frame.magnitude[k] = sqrtf(re*re + im*im);

        float phase       = atan2f(im, re);
        float delta_phase = phase - ctx->prev_phase[k] - hop_angle * k;

        while (delta_phase >  M_PI) delta_phase -= 2.0f * M_PI;
        while (delta_phase < -M_PI) delta_phase += 2.0f * M_PI;

        ctx->frame.frequency[k] = (k * bin_freq) +
                                   (delta_phase / hop_angle) * bin_freq;
        ctx->frame.phase[k]     = phase;
        ctx->prev_phase[k]      = phase;
    }
}

static void _pick_peaks(ProbeContext *ctx)
{
    int   indices[NUM_OSCILLATORS]    = {0};
    float magnitudes[NUM_OSCILLATORS] = {0};

    for (int k = 2; k < FFT_SIZE/2 - 1; k++)
    {
        float m = ctx->frame.magnitude[k];

        if (m < ctx->frame.magnitude[k-1] ||
            m < ctx->frame.magnitude[k+1])
            continue;

        for (int j = 0; j < NUM_OSCILLATORS; j++)
        {
            if (m > magnitudes[j])
            {
                memmove(&magnitudes[j+1], &magnitudes[j],
                        (NUM_OSCILLATORS-j-1) * sizeof(float));
                memmove(&indices[j+1], &indices[j],
                        (NUM_OSCILLATORS-j-1) * sizeof(int));
                magnitudes[j] = m;
                indices[j]    = k;
                break;
            }
        }
    }

    for (int j = 0; j < NUM_OSCILLATORS; j++)
    {
        ctx->oscs[j].target_freq = ctx->frame.frequency[indices[j]];
        ctx->oscs[j].target_amp  = ctx->frame.magnitude[indices[j]] / (FFT_SIZE/2);
        ctx->oscs[j].active      = (magnitudes[j] > 0.001f);
    }
}

static void _spectral_transform(ProbeContext *ctx)
{
    if (ctx->spectral.pitch_shift != 0.0f)
    {
        float ratio = powf(2.0f, ctx->spectral.pitch_shift / 12.0f);
        for (int k = 0; k < FFT_SIZE/2; k++)
            ctx->frame.frequency[k] *= ratio;
    }

    if (ctx->spectral.freeze > 0.0f)
    {
        if (!ctx->have_freeze) {
            memcpy(ctx->frozen_mag, ctx->frame.magnitude,
                   sizeof(ctx->frozen_mag));
            ctx->have_freeze = 1;
        }
        for (int k = 0; k < FFT_SIZE/2; k++)
            ctx->frame.magnitude[k] =
                ctx->frame.magnitude[k] * (1.0f - ctx->spectral.freeze)
              + ctx->frozen_mag[k]       *         ctx->spectral.freeze;
    }
    else {
        ctx->have_freeze = 0;
    }

    if (ctx->spectral.smear > 0.0f)
    {
        float smeared[FFT_SIZE/2];
        float w = ctx->spectral.smear;
        smeared[0] = ctx->frame.magnitude[0];
        for (int k = 1; k < FFT_SIZE/2 - 1; k++)
            smeared[k] = (1.0f - w) * ctx->frame.magnitude[k]
                       + (w / 2.0f) * (ctx->frame.magnitude[k-1]
                                     + ctx->frame.magnitude[k+1]);
        memcpy(ctx->frame.magnitude, smeared, sizeof(smeared));
    }
}

static float _osc_bank_process(ProbeContext *ctx)
{
    float out = 0.0f;
    float phase_inc_scale = SINE_TABLE_SIZE / SAMPLE_RATE;

    for (int j = 0; j < NUM_OSCILLATORS; j++)
    {
        if (!ctx->oscs[j].active) continue;

        ctx->oscs[j].frequency += 0.01f *
            (ctx->oscs[j].target_freq - ctx->oscs[j].frequency);
        ctx->oscs[j].amplitude += 0.01f *
            (ctx->oscs[j].target_amp  - ctx->oscs[j].amplitude);

        ctx->oscs[j].phase += ctx->oscs[j].frequency * phase_inc_scale;
        if (ctx->oscs[j].phase >= SINE_TABLE_SIZE)
            ctx->oscs[j].phase -= SINE_TABLE_SIZE;

        int   idx  = (int)ctx->oscs[j].phase;
        float frac = ctx->oscs[j].phase - idx;
        float s    = ctx->sine_table[idx] +
                     frac * (ctx->sine_table[(idx+1) % SINE_TABLE_SIZE]
                           - ctx->sine_table[idx]);

        out += s * ctx->oscs[j].amplitude;
    }

    return out / NUM_OSCILLATORS;
}

static float _lpg_process(ProbeContext *ctx, float input)
{
    // Envelope follower on input to drive the gate
    float rect = fabsf(input);
    if (rect > ctx->lpg.env)
        ctx->lpg.env += ctx->lpg.attack  * (rect - ctx->lpg.env);
    else
        ctx->lpg.env += ctx->lpg.release * (rect - ctx->lpg.env);

    float cutoff = 200.0f + ctx->lpg.env * 8000.0f;
    float w = cutoff / SAMPLE_RATE * 2.0f * M_PI;
    float k = ctx->lpg.resonance;

    float v1 = (input - (k + w) * ctx->lpg.z1 - ctx->lpg.z2)
             / (1.0f + w * (k + w));
    float v2 = ctx->lpg.z1 + w * v1;
    float v3 = ctx->lpg.z2 + w * v2;

    ctx->lpg.z1 = 2.0f * v2 - ctx->lpg.z1;
    ctx->lpg.z2 = 2.0f * v3 - ctx->lpg.z2;

    return v3 * ctx->lpg.env;
}

// Precomputed sine table
#define SINE_TABLE_SIZE 1024
float sine_table[SINE_TABLE_SIZE];

void osc_bank_init(void)
{
    for (int i = 0; i < SINE_TABLE_SIZE; i++)
        sine_table[i] = sinf(2.0f * M_PI * i / SINE_TABLE_SIZE);
}

float osc_bank_process(Oscillator *oscs, int n_oscs)
{
    float out = 0.0f;
    float phase_inc_scale = SINE_TABLE_SIZE / 48000.0f;

    for (int j = 0; j < n_oscs; j++)
    {
        if (!oscs[j].active) continue;

        // Smooth frequency and amplitude toward targets
        oscs[j].frequency += 0.01f * (oscs[j].target_freq - oscs[j].frequency);
        oscs[j].amplitude += 0.01f * (oscs[j].target_amp  - oscs[j].amplitude);

        // Advance phase
        oscs[j].phase += oscs[j].frequency * phase_inc_scale;
        if (oscs[j].phase >= SINE_TABLE_SIZE)
            oscs[j].phase -= SINE_TABLE_SIZE;

        // Table lookup with linear interpolation
        int   idx  = (int)oscs[j].phase;
        float frac = oscs[j].phase - idx;
        float s    = sine_table[idx] +
                     frac * (sine_table[(idx+1) % SINE_TABLE_SIZE]
                             - sine_table[idx]);

        out += s * oscs[j].amplitude;
    }

    return out / n_oscs;  // normalise
}


void probe_process(ProbeContext *ctx,
                   const int32_t *rx, int32_t *tx, uint32_t size)
{
    for (uint32_t i = 0; i < size; i += 2)
    {
        // Take left channel input, normalise to float
        float input = (float)rx[i] / 2147483648.0f;

        // Feed hop accumulator
        ctx->hop_buf[ctx->hop_counter++] = input;

        // When we have a full hop, run the analysis pipeline
        if (ctx->hop_counter >= HOP_SIZE)
        {
            ctx->hop_counter = 0;

            // Shift analysis buffer and append new hop
            memmove(ctx->analysis_buf,
                    ctx->analysis_buf + HOP_SIZE,
                    (FFT_SIZE - HOP_SIZE) * sizeof(float));
            memcpy(ctx->analysis_buf + FFT_SIZE - HOP_SIZE,
                   ctx->hop_buf,
                   HOP_SIZE * sizeof(float));

            // Run the pipeline
            _analyse_frame(ctx);        // FFT + phase analysis
            _spectral_transform(ctx);   // pitch shift, freeze, smear
            _pick_peaks(ctx);           // select strongest bins
                                        // updates osc targets
        }

        // Oscillator bank runs every sample
        float synth = _osc_bank_process(ctx);

        // LPG shapes the output
        float out = _lpg_process(ctx, synth);

        // Write stereo output
        tx[i]     = (int32_t)(out * 2147483648.0f);
        tx[i + 1] = (int32_t)(out * 2147483648.0f);
    }
}
