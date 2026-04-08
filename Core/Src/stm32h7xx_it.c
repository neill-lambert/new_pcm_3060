/**
  ******************************************************************************
  * @file    Templates/Src/stm32h7xx_it.c
  * @author  MCD Application Team
  * @brief   Main Interrupt Service Routines for Cortex-M7.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32h7xx_it.h"

/** @addtogroup STM32H7xx_HAL_Examples
  * @{
  */

/** @addtogroup BSP
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* Audio Buffers */
#define AUDIO_BUFFER_SIZE 512
extern int32_t tx_buffer[AUDIO_BUFFER_SIZE];
extern int32_t rx_buffer[AUDIO_BUFFER_SIZE];

volatile uint8_t audio_buffer_ready = 0;
volatile uint32_t tx_irq_count = 0;
volatile uint32_t rx_irq_count = 0;
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M7 Processor Exceptions Handlers                         */
/******************************************************************************/

void WWDG1_IRQHandler(void)
{
	__asm("nop");
}

void RCC_IRQHandler(void)
{
    // clear all RCC interrupt flags
    RCC->CICR = 0xFFFFFFFF;
}
/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{

}

/******************************************************************************/
/*                 STM32H7xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32h7xx.s).                                               */
/******************************************************************************/


/**
  * @brief  This function handles DMA1 Stream 0 interrupt (TX).
  */
void DMA_STR0_IRQHandler(void)
{
    LL_GPIO_TogglePin(GPIOE, LL_GPIO_PIN_1);  // any free GPIO
  if (LL_DMA_IsActiveFlag_HT0(DMA1))
  {
    LL_DMA_ClearFlag_HT0(DMA1);
    /* Clean the first half of the TX buffer to ensure DMA sees CPU updates */
    SCB_CleanDCache_by_Addr((uint32_t *)&tx_buffer[0], (AUDIO_BUFFER_SIZE / 2) * sizeof(int32_t));
    audio_buffer_ready = 1;
    // SYNC COPY: Copy First Half
    for(int i = 0; i < 256; i++)
	   {
		   tx_buffer[i] = rx_buffer[i];
	   }

  }
  if (LL_DMA_IsActiveFlag_TC0(DMA1))
  {
    LL_DMA_ClearFlag_TC0(DMA1);
    /* Clean the second half of the TX buffer to ensure DMA sees CPU updates */
    SCB_CleanDCache_by_Addr((uint32_t *)&tx_buffer[AUDIO_BUFFER_SIZE / 2], (AUDIO_BUFFER_SIZE / 2) * sizeof(int32_t));
    audio_buffer_ready = 2;
    // SYNC COPY: Copy Second Half
   for(int i = 256; i < 512; i++)
	   {
		   tx_buffer[i] = rx_buffer[i];
	   }
    tx_irq_count++;
  }
}

/**
  * @brief  This function handles DMA1 Stream 1 interrupt (RX).
  */
void DMA_STR1_IRQHandler(void)
{
  if (LL_DMA_IsActiveFlag_HT1(DMA1))
  {
    LL_DMA_ClearFlag_HT1(DMA1);
    /* Invalidate the first half of the RX buffer to ensure CPU sees DMA updates */
    SCB_InvalidateDCache_by_Addr((uint32_t *)&rx_buffer[0], (AUDIO_BUFFER_SIZE / 2) * sizeof(int32_t));
//    uint32_t sample = rx_buffer[0];
//	if (sample > 0x00FFFFFF && sample < 0xFF000000) myAlign = left;
//	else myAlign = right;
  }
  if (LL_DMA_IsActiveFlag_TC1(DMA1))
  {
    LL_DMA_ClearFlag_TC1(DMA1);
    /* Invalidate the second half of the RX buffer to ensure CPU sees DMA updates */

    SCB_InvalidateDCache_by_Addr((uint32_t *)&rx_buffer[AUDIO_BUFFER_SIZE / 2], (AUDIO_BUFFER_SIZE / 2) * sizeof(int32_t));
    rx_irq_count++;
//    uint32_t sample = rx_buffer[0];
//	if (sample > 0x00FFFFFF && sample < 0xFF000000) myAlign = left;
//	else myAlign = right;
  }
}


/**
  * @}
  */

/**
  * @}
  */

