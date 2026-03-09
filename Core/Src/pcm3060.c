#include "pcm3060.h"
#include "stm32h7xx_ll_utils.h"

/**
 * @brief Write data to PCM3060 register via SPI
 */
void PCM3060_WriteReg(SPI_TypeDef *SPIx, uint8_t reg, uint8_t data) {
    // Wait for TXP (Transmit Packet) space
    while (!LL_SPI_IsActiveFlag_TXP(SPIx));
    
    // Write data to TXDR (16-bit or 8-bit access depends on configuration, but here we do 8-bit)
    LL_SPI_TransmitData8(SPIx, reg & 0x7F);
    while (!LL_SPI_IsActiveFlag_TXP(SPIx));
    LL_SPI_TransmitData8(SPIx, data);
    
    // Wait for transfer complete
    while (!LL_SPI_IsActiveFlag_EOT(SPIx));
    LL_SPI_ClearFlag_EOT(SPIx);
    LL_SPI_ClearFlag_TXTF(SPIx);
}

void PCM3060_Init(SPI_TypeDef *SPIx) {
    // Enable SPI
    LL_SPI_Enable(SPIx);
    // Start Master Transfer
    LL_SPI_StartMasterTransfer(SPIx);

    // Perform Software Reset (MRST)
    PCM3060_WriteReg(SPIx, PCM3060_REG_MRST_ADPS, 0x00);
    LL_mDelay(10); 

    // Release Reset and set Dual Speed mode (ADPS/DAPS = 01)
    PCM3060_WriteReg(SPIx, PCM3060_REG_MRST_ADPS, 0xD2); 

    // Configure DAC Control 1: I2S format, 24-bit
    PCM3060_WriteReg(SPIx, PCM3060_REG_DAC_CTRL1, 0x01); 

    // Configure ADC Control 1: I2S format, 24-bit
    PCM3060_WriteReg(SPIx, PCM3060_REG_ADC_CTRL1, 0x01); 
}
