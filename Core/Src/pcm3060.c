#include "pcm3060.h"

#include "stm32h7xx_ll_gpio.h"
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
    SPIx->IFCR = 0xFFFFFFFF;
    LL_SPI_SetTransferDirection(SPIx, LL_SPI_SIMPLEX_TX);    LL_SPI_SetTransferSize(SPIx, 2); //maybe adjust tx dir to simplextx
    LL_SPI_Enable(SPIx);
    // CS low
    LL_GPIO_ResetOutputPin(GPIOD, LL_GPIO_PIN_14);
    SPI4->IFCR |= SPI_IFCR_MODFC;       // 1. Clear any old fault
    //SPI4->CFG2 |= SPI_CFG2_SP_0;      // ti or motorola std? not sure. leave as moto

    SPI4->CR1  |= SPI_CR1_SSI;          // 2. Force internal signal HIGH
    SPI4->CFG2 |= SPI_CFG2_SSM;         // 3. Enable Software Management
    SPI4->CFG2 |= SPI_CFG2_MASTER;      // 4. NOW set Master mode
    SPI4->CR1  |= SPI_CR1_SPE;          // 5. Enable SPI

    //now line up data to xfer.
    // write both bytes
    while (!(SPI4->SR & SPI_SR_TXP));
    *(volatile uint8_t *)&SPI4->TXDR = reg & 0x7F; //important cast. leave. otherwise spi starts assuming to look for 32bit frames.
    while (!(SPI4->SR & SPI_SR_TXP));
    *(volatile uint8_t *)&SPI4->TXDR = data;
    // start
    SPI4->CR1 |= SPI_CR1_CSTART;
    __DSB();
    while(!LL_SPI_IsActiveFlag_EOT(SPIx));
    LL_SPI_ClearFlag_EOT(SPIx);
    __DSB();
    //CS high
    LL_GPIO_SetOutputPin(GPIOD, LL_GPIO_PIN_14);
}
//
//void PCM3060_WriteReg(SPI_TypeDef *SPIx, uint8_t reg, uint8_t data) {
//    pcm3060_shadow[PCM3060_REG_IDX(reg)] = data;
//
//    // Ensure SPI is disabled to configure
//    LL_SPI_Disable(SPIx);
//    SPIx->IFCR = 0xFFFFFFFF;
//
//    // Set transfer size BEFORE enable
//    LL_SPI_SetTransferSize(SPIx, 2);
//
//    // Enable
//    LL_SPI_Enable(SPIx);
//
//    // CS low
//    LL_GPIO_ResetOutputPin(GPIOD, LL_GPIO_PIN_14);
//
//    // Load both bytes
//    while (!(SPIx->SR & SPI_SR_TXP));
//    *(volatile uint8_t *)&SPIx->TXDR = reg & 0x7F;
//    while (!(SPIx->SR & SPI_SR_TXP));
//    *(volatile uint8_t *)&SPIx->TXDR = data;
//
//    // Start transfer
//    SPIx->CR1 |= SPI_CR1_CSTART;
//
//    // Wait for completion
//    while (!LL_SPI_IsActiveFlag_EOT(SPIx));
//    LL_SPI_ClearFlag_EOT(SPIx);
//
//    // CS high
//    LL_GPIO_SetOutputPin(GPIOD, LL_GPIO_PIN_14);
//}

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
   // PCM3060_ClearBit(SPIx, 64, 7);
    //__DSB();
    //PCM3060_ClearBit(SPIx, 64, 6);
    //__DSB();
    //LL_mDelay(10);

    //PCM3060_WriteReg(SPIx, PCM3060_REG_MRST_ADPS, 0xC0); //check this val???
    //LL_mDelay(10);
    // Release Reset and set Dual Speed mode (ADPS/DAPS = 01)
//    PCM3060_WriteReg(SPIx, PCM3060_REG_MRST_ADPS, 0x00);
//    __DSB();
//
//    PCM3060_WriteReg(SPIx, PCM3060_REG_MRST_ADPS, 0xFF);
//    __DSB();


    PCM3060_WriteReg(SPIx, PCM3060_REG_MRST_ADPS, 0xC0);
    __DSB();

    PCM3060_WriteReg(SPIx, PCM3060_REG_CLOCK_SEL, 0x00);
    __DSB();
    //configure dac ctrl1. attenuation l. all ffs. no at.
    PCM3060_WriteReg(SPIx, PCM3060_REG_DAC_ATTEN_L, 0xFF);

    //configure dac ctrl1. attenuation r. all ffs. no at.
    PCM3060_WriteReg(SPIx, PCM3060_REG_DAC_ATTEN_R, 0xFF);

    // Configure ADC Control 2: attenuation l. 1101 0111b 215 = 0db.
    PCM3060_WriteReg(SPIx, PCM3060_REG_ADC_ATTEN_L, 0xD7);

    // Configure ADC Control 2: attenuation r. 1101 0111b 215 = 0db.
    PCM3060_WriteReg(SPIx, PCM3060_REG_ADC_ATTEN_R, 0xD7);

    // unmute softmutes.
    PCM3060_WriteReg(SPIx, PCM3060_REG_DAC_OS_S_MUTE, 0x00);

    //config wait until txe empty here.
}
