/*
 * probe_processing.H
 *
 *  Created on: May 29, 2026
 *      Author: neill-lambert
 */



#ifndef __PROBE_PROCESSING_H
#define __PROBE_PROCESSING_H

#include <stdint.h>

// Opaque forward declaration — caller never sees internals
typedef struct ProbeContext ProbeContext;

// Lifecycle
ProbeContext *probe_init(void);
void          probe_free(ProbeContext *ctx);

// Control
void probe_trigger(ProbeContext *ctx);
void probe_set_pitch_shift(ProbeContext *ctx, float semitones);
void probe_set_freeze(ProbeContext *ctx, float amount);
void probe_set_smear(ProbeContext *ctx, float amount);
void probe_set_resonance(ProbeContext *ctx, float amount);
void probe_set_decay(ProbeContext *ctx, float ms);

// Audio — call from your DMA IRQ handler
void probe_process(ProbeContext *ctx,
                   const int32_t *rx, int32_t *tx, uint32_t size);

#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
}
#endif

#endif /* __PROBE_PROCESSING_H */
