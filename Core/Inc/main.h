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


void Error_Handler(void);

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
