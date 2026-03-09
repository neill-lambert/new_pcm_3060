#ifndef PCM3060_H
#define PCM3060_H

#include "stm32h7xx_ll_spi.h"

/* PCM3060 Registers */
#define PCM3060_REG_MRST_ADPS    64
#define PCM3060_REG_DAC_CTRL1    65
#define PCM3060_REG_DAC_CTRL2    66
#define PCM3060_REG_DAC_ATTEN_L  67
#define PCM3060_REG_DAC_ATTEN_R  68
#define PCM3060_REG_ADC_CTRL1    69
#define PCM3060_REG_ADC_CTRL2    70
#define PCM3060_REG_ADC_ATTEN_L  71
#define PCM3060_REG_ADC_ATTEN_R  72

/* PCM3060 Configuration Functions */
void PCM3060_Init(SPI_TypeDef *SPIx);
void PCM3060_WriteReg(SPI_TypeDef *SPIx, uint8_t reg, uint8_t data);

#endif /* PCM3060_H */
