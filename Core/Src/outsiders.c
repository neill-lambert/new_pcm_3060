///*
// * outsiders.c
// *
// *  Created on: Mar 18, 2026
// *      Author: neill-lambert
// */
//
//
////enable clocks to important places
// RCC->CKGAENR = 0;    //TODO: clock gate most things
// RCC->AHB1ENR = RCC_AHB1ENR_DMA1EN | RCC_AHB1ENR_CRCEN;
// RCC->AHB2ENR = RCC_AHB2ENR_RNGEN | RCC_AHB2ENR_AHBSRAM2EN | RCC_AHB2ENR_AHBSRAM1EN;
// RCC->AHB3ENR = RCC_AHB3ENR_IOMNGREN | RCC_AHB3ENR_OSPI2EN | RCC_AHB3ENR_FMCEN | RCC_AHB3ENR_GFXMMUEN;
// RCC->AHB4ENR = RCC_AHB4ENR_GPIOAEN | RCC_AHB4ENR_GPIOBEN | RCC_AHB4ENR_GPIOCEN | RCC_AHB4ENR_GPIODEN | RCC_AHB4ENR_GPIOEEN | RCC_AHB4ENR_GPIOFEN | RCC_AHB4ENR_GPIOGEN | RCC_AHB4ENR_GPIOHEN | RCC_AHB4ENR_GPIOIEN | RCC_AHB4ENR_GPIOJEN | RCC_AHB4ENR_GPIOKEN;
// RCC->APB1LENR = RCC_APB1LENR_SPI2EN | RCC_APB1LENR_SPI3EN | RCC_APB1LENR_UART4EN | RCC_APB1LENR_TIM2EN | RCC_APB1LENR_TIM5EN;
// RCC->APB2ENR = RCC_APB2ENR_SPI1EN;
// RCC->APB4ENR = RCC_APB4ENR_SYSCFGEN;
//
// //set flash wait states to 6
// FLASH->ACR = 0x36;    //for 280mhz
//
// //begin underdocumented SHIT
// #define PWR_CR3_SMPSLEVEL    0x30
// #define PWR_CR3_SMPSEXTHP    0x08
// #define PWR_CR3_SMPSEN        0x04
//
// PWR->CR3 = (PWR->CR3 &~ (PWR_CR3_SMPSLEVEL | PWR_CR3_SMPSEXTHP | PWR_CR3_SMPSEN | PWR_CR3_LDOEN | PWR_CR3_BYPASS)) | PWR_CR3_SMPSEN;
// while(!(PWR->CSR1 & PWR_CSR1_ACTVOSRDY));
// //end underdocumented shit
//
// //VOS0
// PWR->SRDCR = PWR_SRDCR_VOS_1 | PWR_SRDCR_VOS_0;
// while(!(PWR->SRDCR & PWR_SRDCR_VOSRDY));
//
// //first go to safe settings: HSI
// RCC->CR = RCC_CR_HSION;                                                    //HSI on, PLL off
// RCC->CFGR = (RCC->CFGR &~ RCC_CFGR_SW_Msk) | RCC_CFGR_SW_HSI;            //switch to HSI
//
// //set up PLL1 to use HSI/32 = 2MHz as reference
// RCC->PLLCKSELR = RCC_PLLCKSELR_DIVM1_5 | RCC_PLLCKSELR_PLLSRC_HSI;
//
// //configure PLL1.P output, PLL1's range, produce 560MHz, output that over 2
// //PLL1.Q feeds spi units 1 2 and 3, same speed
// RCC->PLLCFGR = RCC_PLLCFGR_DIVP1EN | RCC_PLLCFGR_DIVQ1EN | RCC_PLLCFGR_PLL1RGE_0;
// RCC->PLL1DIVR = ((CPU_CLOCK_RATE / 1000000) << RCC_PLL1DIVR_N1_Pos) | (1 << RCC_PLL1DIVR_P1_Pos) | (1 << RCC_PLL1DIVR_Q1_Pos) | (1 << RCC_PLL1DIVR_R1_Pos);
//
// //turn it on
// RCC->CR |= RCC_CR_PLL1ON;
//
// //while it is coming online, set up clock prescalers (all APBs at 140, all AHBs/AXI/CPU at 280)
// RCC->CDCFGR1 = RCC_CDCFGR1_CDPPRE_0;
// RCC->CDCFGR2 = RCC_CDCFGR2_CDPPRE1_0 | RCC_CDCFGR2_CDPPRE2_0;
// RCC->SRDCFGR = RCC_SRDCFGR_SRDPPRE_2;
//
// //wait for PLL to stabilize and then switch to it
// while (!(RCC->CR & RCC_CR_PLL1RDY));
// RCC->CFGR = (RCC->CFGR &~ RCC_CFGR_SW_Msk) | RCC_CFGR_SW_PLL1;            //switch to PLL
