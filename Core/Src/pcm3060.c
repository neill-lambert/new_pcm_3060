#include "pcm3060.h"
#include "stm32h7xx_ll_utils.h"

/**
 * @brief Write data to PCM3060 register via SPI
 */
void PCM3060_WriteReg(SPI_TypeDef *SPIx, uint8_t reg, uint8_t data) {

    LL_SPI_Disable(SPIx);
    SPIx->IFCR = 0xFFFFFFFF; // Clear EOT, TXC, and all error flags

    // 2. Configure while disabled
    LL_SPI_SetTransferSize(SPIx, 1);
    LL_SPI_SetTransferDirection(SPIx, LL_SPI_SIMPLEX_TX);
    // 3. Enable the peripheral
    LL_SPI_Enable(SPIx);

    // 4. STEP 1: START THE MASTER FIRST (Open the session)
    LL_SPI_StartMasterTransfer(SPIx);

    // 5. STEP 2: NOW LOAD THE DATA (Fill the session)
    LL_SPI_TransmitData8(SPIx, reg & 0x7F);

    // 6. STEP 3: WAIT FOR THE COUNTER
    //while(LL_SPI_GetTransferSize(SPIx) > 0);

    // 7. STEP 4: WAIT FOR EOT
    while(!LL_SPI_IsActiveFlag_EOT(SPIx));

    // 8. STEP 5: CLEAR EOT FOR NEXT TIME
    LL_SPI_ClearFlag_EOT(SPIx);
}

void PCM3060_Init(SPI_TypeDef *SPIx) {
    // Perform Software Reset (MRST) (register 64?)

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
