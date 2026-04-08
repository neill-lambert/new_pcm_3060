#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32h7xx.h"
#include "stm32h7xx_ll_rcc.h"
#include "stm32h7xx_ll_gpio.h"
#include "stm32h7xx_ll_spi.h"
#include "stm32h7xx_ll_dma.h"
#include "stm32h7xx_ll_pwr.h"
#include "stm32h7xx_ll_utils.h"
#include "stm32h7xx_ll_system.h"
#include "stm32h7xx_it.h"

static void DWT_Delay_ms(uint32_t ms);


typedef enum {
	left,
	right
}alignment;

volatile extern alignment myAlign;

void Error_Handler(void);

#define PCM3060_RST_Pin LL_GPIO_PIN_15
#define PCM3060_RST_GPIO_Port GPIOE
#define PCM3060_MODE_Pin LL_GPIO_PIN_11
#define PCM3060_MODE_GPIO_Port GPIOE

static void DWT_Delay_ms(uint32_t ms)
{
	uint32_t start = DWT->CYCCNT;
    while((DWT->CYCCNT - start) < 235000);  // exactly 1ms at 235MHz}
}

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
