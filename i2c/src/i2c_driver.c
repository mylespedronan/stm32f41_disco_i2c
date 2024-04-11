/**
 * This mmodule contains code pertaining to the I2C driver without the use of the HAL library.
 * I2C1 is enabled with PB8 as SCL and PB9 as SDA
*/

#include "../inc/i2c_driver.h"
#include "stm32f4xx.h"
 
/**
 * @brief       Enable I2C1 (PB8 SCL/PB9 SDA)
*/
void I2C_init(void)
{    
    // 1. Enable I2C clock and GPIO clock
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;

    // 2. Configure I2C pins for alternative functions
    //     a. Select alternative function in moder register
    GPIOB->MODER |= (GPIO_MODER_MODER8_1) | (GPIO_MODER_MODER9_1);
    
    //     b. Select open drain output
    GPIOB->OTYPER |= (GPIO_OTYPER_OT8) | (GPIO_OTYPER_OT9);

    //     c. Select high speed for pins
    GPIOB->OSPEEDR |= (GPIO_OSPEEDR_OSPEED8) | (GPIO_OSPEEDR_OSPEED9);

    //     d. Select pull-up for both pins
    GPIOB->PUPDR |= (GPIO_PUPDR_PUPD8_0) | (GPIO_PUPDR_PUPD9_0);

    //     e. Configure alternate function in AFR register (low/high)
    GPIOB->AFR[1] |= (GPIO_AFRH_AFSEL8_2) | (GPIO_AFRH_AFSEL9_2);

    // 3. Reset I2C
    I2C1->CR1 |= (1u << 15);
    I2C1->CR1 &= ~(1u << 15);

    // 4. Program peripheral input clock in I2C_CR2 register in order
    //    to generate correct timings
    I2C1->CR2 |= (45 << 0); // PLCK1 Frequency running at 45 MHz

    // 5. Configure the clock control registers (CCR)
    // Table 61. I2C characteristics give values for T_r(SCL) and T_w(SCLH)
    // Using SM mode with the formula for T_high 
    // where T_high = T_r(SCL) + T_w(SCLH) and T_PCLK1 = 1/45 MHz:
    //      T_high = CCR * T_PCLK1
    //      CCR = T_high / T_PCLK1
    //      CCR = (T_r(SCL) + T_w(SCLH)) / T_PCLK1
    //      CCR = (1000ns + 4000ns) / (22.222ns)
    //      CCR = 225
    
    I2C1->CCR = (225 << 0);                  // Standard Mode | Duty Mode

    // 6. Configure the rise time register
    // Formula for TRISE:
    //      TRISE = (T_r(SCL) / T_PCLK1) + 1
    //      TRISE = (1000ns / 22.22ns) + 1
    //      TRISE = 45 + 1 = 46
    I2C1->TRISE = 46;

    // 7. Program the I2C_CR1 register to enable the peripheral
    I2C1->CR1 |= (1u << 0);   
}

/**
 * @brief       Start an I2C transfer
*/
void I2C_start(void)
{
    // 1. Set the start bit in the I2C_CR1 register to generate Start condition
    I2C1->CR1 |= (1u << 8);                // Generate Start

    // 2. Wait for the start bit (SB, bit 0 in SR1) to set. This indicates that the start 
    //    condition is generated
    while (!(I2C1->SR1 & (1u << 0)));      // Wait for SB to set
}

/**
 * @brief       End an I2C transfer
*/
void I2C_stop(void)
{
    // 1. Stop generation by writing to the STOP register (bit 9 in CR1)
    I2C1->CR1 |= (1u << 9);
}

/**
 * @brief           Write to the I2C address
 * @param address   I2C address of device (Slave address)
 * @param timeout   Timeout to check if byte transfer finished
 * @return          0 for success/1 for failure
*/
uint8_t I2C_writeSlaveAddress(uint8_t address, uint32_t timeout)
{
    uint32_t counter = 0;

    // 1. Send the Slave Address to the DR register
    I2C1->DR = address;

    // 2. Wait for the Address Bit (ADDR, bit 1 in SR1) to set. This indicates the end of address transmission
    while (!(I2C1->SR1 & (1u << 1))) {
        if (counter >= timeout) {
            return 1;
        }
        counter++;
    }

    // 3. Clear the ADDR by reading the SR1 and SR2
    uint8_t temp = I2C1->SR1 | I2C1->SR2;

    return 0;
}

/**
 * @brief           Write data to I2C device
 * @param data      Data to be written to device
 * @param timeout   Timeout to check if byte transfer finished
 * @return          0 for success/1 for failure
*/
uint8_t I2C_write(uint8_t data, uint32_t timeout)
{
    uint32_t counter = 0;

    // From Figure 164. Transfer sequence diagram for master transmitter
    // 1. Wait for the Data register empty for TX (TXE, bit 7 in SR1) to set. This indicates that the DR is empty
    while (!(I2C1->SR1 & (1u << 7)));      // Wait for TXE bit to set;
    
    // 2. Send the DATA to the DR register
    I2C1->DR = data;

    // 3. Wait for the Byte Transfer Finished (BTF, bit 2 in SR1) to set. 
    //    This indicates the end of LAST DATA transmission
    while (!(I2C1->SR1 & (1u << 2))) {
        if (counter >= timeout) {
            return 1;
        }
        counter++;
    }

    return 0;
}

/**
 * @brief           Write multiple data to I2C device
 * @param data      Data to be written to device
 * @param size      Amount of data to be written
 * @param timeout   Timeout to check if byte transfer finished
 * @return          0 for success/1 for failure
*/
uint8_t I2C_writeMulti(uint8_t *data, uint8_t size, uint32_t timeout)
{
    uint32_t counter = 0;

    // 1. Wait for the Data register empty for TX (TXE, bit 7 in SR1) to set. This indicates that the DR is empty
    while (!(I2C1->SR1 & (1u << 7)));           // Wait for TXE bit to set;

    // 2. Keep sending DATA to the DR register after performing the check if the TXe bit is set
    while (size) {
        while (!(I2C1->SR1 & (1u << 7)));       // Wait for TXE bit to set;
        I2C1->DR = (uint8_t)*data++;   // Send data
        size--;
    }

    // 3. Once the DATA transfer is complete, wait for the BTF (bit 2 in SR1) to set. This indicates the end of
    //    LAST DATA transmission
    while (!(I2C1->SR1 & (1u << 2))) {
        if (counter >= timeout) {
            return 1;
        }
        counter++;        
    };

    return 0;
}
