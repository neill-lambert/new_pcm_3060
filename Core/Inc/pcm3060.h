#ifndef PCM3060_H
#define PCM3060_H

#include "stm32h7xx_ll_spi.h"
#include "main.h"

/* PCM3060 Registers */
#define PCM3060_REG_MRST_ADPS          64
#define PCM3060_REG_DAC_ATTEN_L        65
#define PCM3060_REG_DAC_ATTEN_R        66
#define PCM3060_REG_CLOCK_SEL          67
#define PCM3060_REG_DAC_OS_S_MUTE      68
#define PCM3060_REG_DAC_FILTER         69
#define PCM3060_REG_ADC_ATTEN_L         70
#define PCM3060_REG_ADC_ATTEN_R         71
#define PCM3060_REG_ADC_CLOCK_SEL		72

/* PCM3060 Configuration Functions */
void PCM3060_Init(SPI_TypeDef *SPIx);
void PCM3060_WriteReg(SPI_TypeDef *SPIx, uint8_t reg, uint8_t data);

#endif /* PCM3060_H */
