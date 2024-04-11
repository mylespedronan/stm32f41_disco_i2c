/**
 * This program uses the I2C protocol without the use of the HAL library. The program writes to the 
 * SSD1306 OLED screen and uses GPIO interrupts to move an image left to right. This program also allows
 * for strings or characters to be written to the screen.
*/

#include "RTE_Components.h"
#include "../inc/gpio.h"
#include "../inc/timer.h"
#include "../inc/i2c_driver.h"
#include "../inc/ssd1306_driver.h"

#include CMSIS_device_header

// Wrap text
#define NO_WRAP         0u
#define WRAP            1u

/**
 * @brief   Initialize modules
 * @return  0 for success/1 for failure
*/
uint8_t initialize(void)
{
    uint8_t rv = 0;

    // Initialize system clock and GPIO timer
    rv += SysClockConfig();
    rv += TIM2init();
    
    // Initialize GPIO and GPIO interrupts
    initGPIO();
    initGPIOInterrupt();

    // Init I2C driver
    I2C_init();

    // Init SSD1306 (OLED)   
    rv = SSD1306_init();
    if (rv != 0) {
        return 1;
    }
    
    return rv;
}

int main(void)
{
    uint8_t rv = 0;
    char myText[] = "Hello World!";

    rv = initialize();
    if (rv != 0) {
        return 1;
    }
    
    /////////////////////////////////
    // Set cursor position
    /////////////////////////////////
    // SSD1306_setCursor(5, 5);

    /////////////////////////////////
    // Test Filling screen with color
    /////////////////////////////////
    // SSD1306_fill(WHITE);
    // rv = SSD1306_update();
    // if (rv != 0) {
    //     return 1;
    // }

    /////////////////////////////////
    // Write string to screen
    /////////////////////////////////
    // SSD1306_setCursor(5, 5);
    // SSD1306_writeString(myText, Font_11x18, WHITE, WRAP);
    // rv = SSD1306_update();
    // if (rv != 0) {
    //     return 1;
    // }

    /////////////////////////////////
    // Draw image on screen
    /////////////////////////////////
    // SSD1306_setCursor(10, 10);
    // SSD1306_writeImg(Ryu_32x36, WHITE);
    // rv = SSD1306_update();
    // if (rv != 0) {
    //     return 1;
    // }

    while (1) {
        /////////////////////////////////
        // Draw animation 
        /////////////////////////////////
        SSD1306_setCursor(10, 10);
        SSD1306_writeImg(DogDown_22x20, WHITE);
        SSD1306_update();
        if (rv != 0) {
            return 1;
        }
        Delay_ms(2);

        SSD1306_setCursor(10, 10);
        SSD1306_writeImg(DogUp_22x20, WHITE);
        SSD1306_update();
        if (rv != 0) {
            return 1;
        }
        Delay_ms(2);
    }
    
}