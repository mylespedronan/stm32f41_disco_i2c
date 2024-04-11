/**
 * This module contains code pertaining to the timer driver without the use of the HAL library.
 * Timer 2 is enabled for the GPIO and the System Clock is configured
 * 
 * More details can be found:
 * - https://github.com/weewStack/STM32F1-Tutorial/blob/master/020-STM32F1_DELAY_FUNCTION_SYSTICK_TIMER/main.c
 * - https://www.youtube.com/watch?v=usvAIEdp_I8
*/

#include "stm32f4xx.h"
#include "../inc/timer.h"

// PLL configure values (PLLCFGR)
#define PLL_M       4u          // Division factor for the main PLL input clock
#define PLL_N       180u        // Main PLL multiplication factor for VCO
#define PLL_P       0           // Main PLL division factor for main system clock (PLLP = 2)

// Timeout value
#define TIMER_TIMEOUT    100000u

/**
 * @brief       Initialize system clock
 * @return      0 for success/1 for failure
*/
uint8_t SysClockConfig(void)
{
    uint32_t counter = 0;

    // 1. Enable HSE and wait for the HSE to become ready
    RCC->CR |= RCC_CR_HSEON;
    while (!(RCC->CR & RCC_CR_HSERDY)) {
        if (counter >= TIMER_TIMEOUT) {
            return 1;
        }
        counter++;
    };

    // 2. Set the POWER ENABLE CLOCK and VOLTAGE REGULATOR
    RCC->APB1ENR |= RCC_APB1ENR_PWREN;
    PWR->CR |= PWR_CR_VOS;

    // 3. Configure the FLASH PREFETCH and the LATENCY Related Settings
    FLASH->ACR |= FLASH_ACR_ICEN | FLASH_ACR_DCEN | FLASH_ACR_PRFTEN | FLASH_ACR_LATENCY_5WS;

    // 4. Configure the PRESCALARS HCLK, PCLK1, PCLK2
    //    AHB PR
    RCC->CFGR |= RCC_CFGR_HPRE_DIV1;

    //    APB1 PR
    RCC->CFGR |= RCC_CFGR_PPRE1_DIV4;

    //    APB2 PR
    RCC->CFGR |= RCC_CFGR_PPRE2_DIV2;

    // 5. Configure the MAIN PLL
    RCC->PLLCFGR |= (PLL_M << 0) | (PLL_N << 6) | (PLL_P << 16) | (RCC_PLLCFGR_PLLSRC_HSE);

    // 6. Enable the PLL and wait for it to become ready
    RCC->CR |= RCC_CR_PLLON;
    counter = 0;
    while (!(RCC->CR & RCC_CR_PLLRDY)) {
        if (counter >= TIMER_TIMEOUT) {
            return 1;
        }
        counter++;
    }

    // 7. Select the Clock Source and wait for it to be set
    RCC->CFGR |= RCC_CFGR_SW_PLL;
    counter = 0;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL) {
        if (counter >= TIMER_TIMEOUT) {
            return 1;
        }
        counter++;
    }

    return 0;
}

/**
 * @brief       Initialize Timer 2
 * @return      0 for success/1 for failure
*/
uint8_t TIM2init(void)
{
    uint32_t counter = 0;

    // 1. Enable Timer clock
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

    // 2. Set the prescalar and the ARR
    //    Prescalar formula: F_ck_psc / (PSC[15:0] + 1)
    TIM2->PSC = 90 - 1;     // 90MHz / 90 = 1 MHz ~ 1uS delay
    TIM2->ARR = 0xFFFF;     // MAX ARR Value

    // 3. Enable the Timer, and wait for the update interrupt flag to set
    TIM2->CR1 |= (1u << 0);
    while (!(TIM2->SR & (1u << 0))) {
        if (counter >= TIMER_TIMEOUT) {
            return 1;
        }
        counter++;
    }

    return 0;
}

/**
 * @brief       Delay for x amount of microseconds
 * @param us    Amount of time, in uS, to delay
*/
void Delay_us(uint16_t us)
{
    // 1. Reset the counter
    TIM2->CNT = 0;

    // 2. Wait for the counter to reach the entered value. As each count
    //    will take 1us, the total waiting time will be the required us delay
    while (TIM2->CNT < us);
}

/**
 * @brief       Delay for x amount of miliseconds
 * @param ms    Amount of time, in mS, to delay
*/
void Delay_ms(uint16_t ms)
{
    for (uint16_t i = 0; i < ms; i++) {
        Delay_us(1000);     // Delay of 1 ms
    }
}
