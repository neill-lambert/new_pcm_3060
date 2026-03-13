#include "pcm3060.h"
#include "stm32h7xx_ll_utils.h"

// Indices 0–9 map to registers 64–73
// Values are power-on defaults from the PCM3060 datasheet
static uint8_t pcm3060_shadow[10] = {
    0xF0,  // reg 64 (0x40) — System Control 1 (Reset, Power)
    0xFF,  // reg 65 (0x41) — DAC Attenuation (Left)
    0xFF,  // reg 66 (0x42) — DAC Attenuation (Right)
    0x00,  // reg 67 (0x43) — DAC Control / Format 1
    0x00,  // reg 68 (0x44) — DAC Control / Soft Mute
    0x00,  // reg 69 (0x45) — ADC Control 1
    0xD7,  // reg 70 (0x46) — ADC Attenuation (Left)
    0xD7,  // reg 71 (0x47) — ADC Attenuation (Right)
    0x00,  // reg 72 (0x48) — ADC Control / Format 2
    0x00,  // reg 73 (0x49) — ADC Control / Soft Mute
};

// Helper macro to convert reg address → array index
#define PCM3060_REG_IDX(reg)  ((reg) - 64)

/**
 * @brief Write data to PCM3060 register via SPI
 */
void PCM3060_WriteReg(SPI_TypeDef *SPIx, uint8_t reg, uint8_t data) {

    pcm3060_shadow[PCM3060_REG_IDX(reg)] = data;  // keep shadow in sync
    LL_SPI_Disable(SPIx);
    SPIx->IFCR = 0xFFFFFFFF; // Clear EOT, TXC, and all error flags
    LL_SPI_SetTransferSize(SPIx, 2);
    LL_SPI_SetTransferDirection(SPIx, LL_SPI_SIMPLEX_TX);
    LL_SPI_Enable(SPIx);
    LL_SPI_StartMasterTransfer(SPIx);
    LL_SPI_TransmitData8(SPIx, reg & 0x7F);
    LL_mDelay(10);
    LL_SPI_TransmitData8(SPIx, data);          // actual data value
    while(!LL_SPI_IsActiveFlag_EOT(SPIx));
    LL_SPI_ClearFlag_EOT(SPIx);
}


void PCM3060_ClearBit(SPI_TypeDef *SPIx, uint8_t reg, uint8_t bit) {
    uint8_t val = pcm3060_shadow[PCM3060_REG_IDX(reg)];
    val &= ~(1 << bit);
    PCM3060_WriteReg(SPIx, reg, val);
}

void PCM3060_SetBit(SPI_TypeDef *SPIx, uint8_t reg, uint8_t bit) {
    uint8_t val = pcm3060_shadow[PCM3060_REG_IDX(reg)];
    val |= (1 << bit);
    PCM3060_WriteReg(SPIx, reg, val);
}

void PCM3060_Init(SPI_TypeDef *SPIx) {
    // Perform Software Reset (MRST) (register 64?)
    PCM3060_ClearBit(SPIx, 64, 7);
    PCM3060_ClearBit(SPIx, 64, 6);
    LL_mDelay(10);

    PCM3060_WriteReg(SPIx, PCM3060_REG_MRST_ADPS, 0x00);
    LL_mDelay(10); 

    // Release Reset and set Dual Speed mode (ADPS/DAPS = 01)
    PCM3060_WriteReg(SPIx, PCM3060_REG_MRST_ADPS, 0xD2); 

    // Configure DAC Control 1: I2S format, 24-bit
    PCM3060_WriteReg(SPIx, PCM3060_REG_DAC_CTRL1, 0x01); 

    // Configure DAC Control 2: Unmute DAC
    PCM3060_WriteReg(SPIx, PCM3060_REG_DAC_CTRL2, 0x00);

    // Configure ADC Control 1: I2S format, 24-bit
    PCM3060_WriteReg(SPIx, PCM3060_REG_ADC_CTRL1, 0x01);

    // Configure ADC Control 2: Unmute ADC
    PCM3060_WriteReg(SPIx, PCM3060_REG_ADC_CTRL2, 0x00);

    //config wait until txe empty here.
}
