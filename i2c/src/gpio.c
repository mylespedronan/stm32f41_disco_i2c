#include "../inc/gpio.h"
#include "../inc/ssd1306_driver.h"
#include "stm32f4xx.h"

// Offset for Bit Set/Reset Register
#define BSRR_OFFSET     16u

/**
 * @brief       Initialize GPIO
*/
void initGPIO(void)
{
    // Enable Clock to Port C Pin 7
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;

    // Configure mode to output
    GPIOC->MODER &= ~(GPIO_MODER_MODER7);
    GPIOC->MODER |= GPIO_MODER_MODER7_0;

    // Set up the pin as push/pull
    GPIOC->OTYPER &= ~(1u << 7);
}

/**
 * @brief       Initialize interrupts for PA4 and PA8
 *              Note: Interrupts cannot share the same EXTI line
 * 
 *              1. Enable IRQ Clock (SYSCFGEN)
 *              2. Setup pins as input (Reset, set as input, set at push/pull)
 *              3. Enable interrupt mask registers
 *              4. Enable EXTI (external interrupt) registers
 *              5. Setup interrupt trigger (rising/falling edge)
 *              6. Enable NVIC (disable irq, enable NVIC irq, enable irq)
*/
void initGPIOInterrupt(void)
{
    // Enable IRQ CLock
    // RCC->AHB2ENR |= (1u << 14);
    RCC->AHB2ENR |= RCC_APB2ENR_SYSCFGEN;

    // Setup PA8 as interrupt
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;

    // PA8
    GPIOA->MODER &= ~(GPIO_MODER_MODER8);
    GPIOA->MODER |= (0x00 << GPIO_MODER_MODER8_Pos);
    GPIOA->OTYPER &= ~(1u << 8);

    // PA4
    GPIOA->MODER &= ~(GPIO_MODER_MODER4);
    GPIOA->MODER |= (0x00 << GPIO_MODER_MODER4_Pos);
    GPIOA->OTYPER &= ~(1u << 4);

    // Enable Interrupt Mask Register
    SYSCFG->EXTICR[2] &= (SYSCFG_EXTICR3_EXTI8_Msk);
    SYSCFG->EXTICR[1] &= (SYSCFG_EXTICR2_EXTI4_Msk);

    // Enable EXTI (external interrupt) register
    EXTI->IMR |= EXTI_IMR_MR8_Msk | EXTI_IMR_MR4_Msk;

    // Select the Interrupt Trigger
    EXTI->RTSR |= (1u << 8) | (1u << 4);

    // NVIC Enable
    __disable_irq();
    NVIC_EnableIRQ(EXTI4_IRQn);
    NVIC_EnableIRQ(EXTI9_5_IRQn);
    __enable_irq();
}

/**
 * @brief       Toggle the LED (PC7)
*/
void ledToggle(void)
{
    volatile uint32_t i;
    // GPIOC->BSRR = 0x80;
    GPIOC->BSRR = (1u << 7);
    for (i = 0; i < 1000000; i++) {}

    // GPIOC->BSRR = 0x00800000;
    GPIOC->BSRR = (1u << (7u + BSRR_OFFSET));
    for (i = 0; i < 1000000; i++) {}
}

/**
 * @brief       Callback for interrupt on PA8
 *              Used to move the image right
 *              Note: Previously used to set the Bit Set/Reset Register (PC7)
*/
void EXTI9_5_IRQHandler() 
{
    // Clear pending register
    EXTI->PR |= (1u << 8);

    // // Set register
    // GPIOC->BSRR = (1u << 7);

    SSD1306_moveImageRight();
}

/**
 * @brief       Callback for interrupt on PA4
 *              Used to move the image left
 *              Note: Previously used to reset the Bit Set/Reset Register (PC7)
*/
void EXTI4_IRQHandler()
{
    // Clear pending register
    EXTI->PR |= (1u << 4);

    // // Reset register
    // GPIOC->BSRR = (1u << (7u + BSRR_OFFSET));
    SSD1306_moveImageLeft();
}
