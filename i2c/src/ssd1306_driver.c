/**
 * This module initializes the SSD1306 and handles writing to the I2C device without the use of the HAL library.
 * The library, https://github.com/4ilo/ssd1306-stm32HAL, is referenced.
*/
#include "../inc/ssd1306_driver.h"
#include "../inc/i2c_driver.h"
#include "../inc/timer.h"
#include "stm32f4xx.h"

// SSD1306 config
#define SSD1306_I2C_ADDR        0x78            // Slave address: “b0111 1000”
#define SSD1306_WIDTH           128u            // OLED width
#define SSD1306_HEIGHT          64u             // OLED height

// Address increment table
#define SSD1306_WRITE_COMMAND   0x00            // DC 0, RW 0
#define SSD1306_WRITE_DATA      0x40            // DC 1, RW 0

// SSD1306 I2C memory address
#define I2C_MEM_ADD_MSB(__ADDRESS__)    ((uint8_t)((uint16_t)(((uint16_t)((__ADDRESS__) & (uint16_t)0xFF00)) >> 8)))
#define I2C_MEM_ADD_LSB(__ADDRESS__)    ((uint8_t)((uint16_t)((__ADDRESS__) & (uint16_t)0x00FF)))

#define I2C_MEMADD_SIZE_8BIT    0x00000001u     // Used to check if memory address is 8-bit
#define I2C_MEMADD_SIZE_16BIT   0x00000010u     // Used to check if memory address is 16-bit

// Configurable settings
#define IMG_STEP_X              5u              // Amount of steps to move image left/right
#define TIMEOUT_MS              100000u         // Max wait time

// Screenbuffer
static uint8_t SSD1306_Buffer[(SSD1306_WIDTH * SSD1306_HEIGHT) / 8];

// Screen Object
static SSD1306_t SSD1306;
static ImgDef lastImg;

// Local Prototypes
uint8_t SSD1306_write(uint8_t data, uint16_t memAddress, uint16_t memSize);
uint8_t SSD1306_writeMulti(uint8_t *data, uint8_t size, uint16_t memAddress, uint16_t memSize);
void SSD1306_draw_pixel(uint8_t x, uint8_t y, SSD1306_COLOR color);
char SSD1306_write_char(char ch, FontDef Font, SSD1306_COLOR color, uint8_t wrap);

/**
 * @brief   Initialize SSD1306
 * @return  0 for success/1 for failure
*/
uint8_t SSD1306_init(void)
{
    uint8_t rv = 0;

    // Wait for screen to boot
    Delay_ms(100);

    //////////////////
    //// Init LCD ////
    //////////////////

    // Set Display ON/OFF (AFh/AEh)
    rv += SSD1306_write(0xAE, SSD1306_WRITE_COMMAND, 1);    // Set Display off

    // Set Memory Addressing mode (20h)
    rv += SSD1306_write(0x20, SSD1306_WRITE_COMMAND, 1);    // Set Memory Addressing Mode
    rv += SSD1306_write(0x10, SSD1306_WRITE_COMMAND, 1);    // Set to Page Addressing Mode
                                                            // 00: Horizontal Addressing Mode;
                                                            // 01: Vertical Addressing Mode;
                                                            // 10: Page Addressing Mode (RESET);
                                                            // 11: Invalid;
    rv += SSD1306_write(0xB0, SSD1306_WRITE_COMMAND, 1);    // Set the page start address (B0h - B7h)
    rv += SSD1306_write(0x00, SSD1306_WRITE_COMMAND, 1);    // Set low column address for page adressing
    rv += SSD1306_write(0x10, SSD1306_WRITE_COMMAND, 1);    // Set high column address for page adressing

    // Set COM Output Scan Direction (C0h/C8h)
    rv += SSD1306_write(0xC8, SSD1306_WRITE_COMMAND, 1);    // COM output scan from COM63 to COM0

    // Set Display Start Line (40h~7Fh)
    rv += SSD1306_write(0x40, SSD1306_WRITE_COMMAND, 1);    // Set start line address

    // Set contrast control register (81h)
    rv += SSD1306_write(0x81, SSD1306_WRITE_COMMAND, 1);    // Set contrast control register
    rv += SSD1306_write(0xFF, SSD1306_WRITE_COMMAND, 1);    // Set contrast steps from 00h to FFh
                                                            // Output current increases as step value increases

    // Set Segment Re-map (A0h/A1h)
    rv += SSD1306_write(0xA1, SSD1306_WRITE_COMMAND, 1);    // Set segment re-map 0 to 127

    // Set Normal/Inverse Display (A6h/A7h)
    rv += SSD1306_write(0xA6, SSD1306_WRITE_COMMAND, 1);    // Set normal display

    // Set multiplex ratio(1 to 64)
    rv += SSD1306_write(0xA8, SSD1306_WRITE_COMMAND, 1);                    // Set multiplex ratio(1 to 64)
    rv += SSD1306_write(SSD1306_HEIGHT - 1, SSD1306_WRITE_COMMAND, 1);      // Set Value of Multiplex Ratio
    rv += SSD1306_write(0xA4, SSD1306_WRITE_COMMAND, 1);                    // 0xA4: Output follows RAM content
                                                                            // 0xA5: Output ignores RAM content
    
     // Set display offset (D3)
    rv += SSD1306_write(0xD3, SSD1306_WRITE_COMMAND, 1);  // Set display offset
    rv += SSD1306_write(0x00, SSD1306_WRITE_COMMAND, 1);  // No offset

    // Set display clock divide ratio/oscillator frequency (D5h)
    rv += SSD1306_write(0xD5, SSD1306_WRITE_COMMAND, 1);  // Set display clock divide ratio/oscillator frequency
    rv += SSD1306_write(0xF0, SSD1306_WRITE_COMMAND, 1);  // Set divide ratio

    // Set pre-charge period (D9h)
    rv += SSD1306_write(0xD9, SSD1306_WRITE_COMMAND, 1);  // Set pre-charge period
    rv += SSD1306_write(0x22, SSD1306_WRITE_COMMAND, 1);  // Precharge set to 34 counts

    // Set com pins hardware configuration (DAh)
    rv += SSD1306_write(0xDA, SSD1306_WRITE_COMMAND, 1);                                // Set com pins hardware configuration
    rv += SSD1306_write(((0u << 5) | (1u << 4) | (0x02)), SSD1306_WRITE_COMMAND, 1);    // Disable L/R remap | Alt COM config

    // Set V_comh Deselect level (DBh)
    rv += SSD1306_write(0xDB, SSD1306_WRITE_COMMAND, 1);  // Set vcomh
    rv += SSD1306_write(0x20, SSD1306_WRITE_COMMAND, 1);  // 0x20: ~0.77 x Vcc

    // Set Charge Pump Settings (0x8D)
    rv += SSD1306_write(0x8D, SSD1306_WRITE_COMMAND, 1);  // Set DC-DC enable
    rv += SSD1306_write(0x14, SSD1306_WRITE_COMMAND, 1);  // Enable Charge Pump

    // Set Display ON/OFF (AFh/AEh)
    rv += SSD1306_write(0xAF, SSD1306_WRITE_COMMAND, 1);  // Set Display off

    // Check return values
    if (rv != 0) {
        return 1;
    }

    // Clear Screen
    SSD1306_fill(BLACK);

    // Flush buffer
    rv += SSD1306_update();
    if (rv != 0) {
        return 1;
    }

    // Set default values for screen
    SSD1306.xpos = 0;
    SSD1306.ypos = 0;
    
    return 0;
}

/**
 * @brief               Write data to SSD1306
 *                      Writes data to a particular memory address of the SSD1306
 * @param data          Data to be written
 * @param memAddress    Memory address of the internal register of the SSD1306
 * @param memSize       Size of the memory address
 * @return              0 for success/1 for failure
*/
uint8_t SSD1306_write(uint8_t data, uint16_t memAddress, uint16_t memSize)
{
    uint8_t rv = 0;

    I2C_start();

    // Wait for busy
    while (!(I2C1->SR2 & (1u << 1)));

    // Send Slave Address
    rv = I2C_writeSlaveAddress(SSD1306_I2C_ADDR, TIMEOUT_MS);
    if (rv != 0) {
        return rv;
    }

    // Wait for TXE bit to set;
    while (!(I2C1->SR1 & (1u << 7)));

    // Check if memory address is 8 or 16 bit;
    if (memSize == I2C_MEMADD_SIZE_8BIT) {
        // Send LSB
        I2C1->DR = I2C_MEM_ADD_LSB(memAddress);
    } else {
        // Send MSB
        I2C1->DR = I2C_MEM_ADD_MSB(memAddress);

        // Wait for TXE bit to set;
        while (!(I2C1->SR1 & (1u << 7)));
        
        // Send LSB
        I2C1->DR = I2C_MEM_ADD_LSB(memAddress);
    }

    // Wait for TXE bit to set;
    while (!(I2C1->SR1 & (1u << 7)));

    // Write data
    rv = I2C_write(data, TIMEOUT_MS);
    if (rv != 0) {
        return rv;
    }

    I2C_stop();

    return rv;
}

/**
 * @brief               Write multiple data packets to SSD1306
 *                      Writes data packets to a particular memory address of the SSD1306
 * @param data          Pointer to the data to be written
 * @param size          Size of data to be written
 * @param memAddress    Memory address of the internal register of the SSD1306
 * @param memSize       Size of the memory address
 * @return              0 for success/1 for failure
*/
uint8_t SSD1306_writeMulti(uint8_t *data, uint8_t size, uint16_t memAddress, uint16_t memSize)
{
    uint8_t rv = 0;

    I2C_start();

    // Wait for busy
    while (!(I2C1->SR2 & (1u << 1)));
    
    // Send Slave Address
    rv = I2C_writeSlaveAddress(SSD1306_I2C_ADDR, TIMEOUT_MS);
    if (rv != 0) {
        return rv;
    }
    
    // Wait for TXE bit to set;
    while (!(I2C1->SR1 & (1u << 7)));               

    // Check if memory address is 8 or 16 bit;
    if (memSize == I2C_MEMADD_SIZE_8BIT) {
        // Send LSB
        I2C1->DR = I2C_MEM_ADD_LSB(memAddress);
    } else {
        // Send MSB
        I2C1->DR = I2C_MEM_ADD_MSB(memAddress);

        // Wait for TXE bit to set;
        while (!(I2C1->SR1 & (1u << 7)));
        
        // Send LSB
        I2C1->DR = I2C_MEM_ADD_LSB(memAddress);
    }
    
    // Wait for TXE bit to set;
    while (!(I2C1->SR1 & (1u << 7)));

    // Write data
    rv = I2C_writeMulti(data, size, TIMEOUT_MS);
    if (rv != 0) {
        return rv;
    }

    I2C_stop();

    return rv;
}

/**
 * @brief   Updates the SSD1306 by writing the data in the buffer
 *          1. Writes to the page start address
 *          2. Writes to the low column address
 *          3. Writes to the high column address
 *          4. Writes buffer to SSD1306 (Table 9-3: Address increment table)
 * @return  0 for success/1 for failure
*/
uint8_t SSD1306_update(void)
{
    uint8_t rv = 0;

    for (uint8_t i = 0; i < 8; i++) {
        // Writes to the page start address
        rv += SSD1306_write((0xB0 + i), SSD1306_WRITE_COMMAND, 1);

        // Writes to the low column address
        rv += SSD1306_write(0x00, SSD1306_WRITE_COMMAND, 1);
        
        // Writes to the high column address
        rv += SSD1306_write(0x10, SSD1306_WRITE_COMMAND, 1);

        // Writes buffer to SSD1306
        rv += SSD1306_writeMulti(&SSD1306_Buffer[SSD1306_WIDTH * i], SSD1306_WIDTH, SSD1306_WRITE_DATA, 1);
    }

    return rv;
}

/**
 * @brief           Fill SSD1306 buffer with on/off (BLACK (0x00)/WHITE (0xFF))
 * @param color     Color to fill screen WHITE/BLACK
*/
void SSD1306_fill(SSD1306_COLOR color)
{
    for (uint32_t i = 0; i < sizeof(SSD1306_Buffer); i++) {
        SSD1306_Buffer[i] = (color == BLACK) ? 0x00 : 0xFF;
    }
}

/**
 * @brief           Set cursor to x/y positions
 *                  This function also sets the initial x/y positions to be 
 *                  referenced when moving an image
 * @param x         X coordinate to be set
 * @param y         Y coordinate to be set
*/
void SSD1306_setCursor(uint8_t x, uint8_t y)
{
    SSD1306.xpos = x;
    SSD1306.ypos = y;

    SSD1306.xpos_init = x;
    SSD1306.ypos_init = y;
}

/**
 * @brief   Moves the current image to the right
 *          Used when an interrupt to move right occurs
*/
void SSD1306_moveImageRight(void)
{
    SSD1306.xpos = SSD1306.xpos_init + IMG_STEP_X;
    if (SSD1306.xpos >= SSD1306_WIDTH) {
        SSD1306.xpos = SSD1306_WIDTH - IMG_STEP_X;
    }

    SSD1306_fill(BLACK);
    SSD1306_writeImg(lastImg, WHITE);
    (void)SSD1306_update();
}

/**
 * @brief   Moves the current image to the left
 *          Used when an interrupt to move left occurs
*/
void SSD1306_moveImageLeft(void)
{
    SSD1306.xpos = SSD1306.xpos_init - IMG_STEP_X;
    if ((SSD1306.xpos <= 0) || (SSD1306.xpos >= SSD1306_WIDTH)){
        SSD1306.xpos = 0 + IMG_STEP_X;
    }

    SSD1306_fill(BLACK);
    SSD1306_writeImg(lastImg, WHITE);
    (void)SSD1306_update();
}

/**
 * @brief           Draw one pixel in the screenbuffer
 * @param x         X coordinate
 * @param y         Y coordinate
 * @param color     Color to fill screen WHITE/BLACK
*/
void SSD1306_draw_pixel(uint8_t x, uint8_t y, SSD1306_COLOR color)
{
    
    // Check if coordinates are outside the buffer
    if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT) {
        // If coordinates are outside the bounds, don't write to screen
        return;
    }

    // Draw in the correct color
    if (color == WHITE) {
        SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] |= 1 << (y % 8);
    } else {
        SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] &= ~(1 << (y % 8));
    }
}

/**
 * @brief           Draw 1 char to the screen buffer
 * @param ch        Character to write to the screen
 * @param Font      Font struct with font parameters
 * @param color     Color to fill screen WHITE/BLACK
 * @param wrap      Used to check if the text needs to wrap
 * @return          Return char being written
*/
char SSD1306_write_char(char ch, FontDef Font, SSD1306_COLOR color, uint8_t wrap)
{
    uint16_t pixel;

    // Check remaining space on current line
    if ((SSD1306_WIDTH <= (SSD1306.xpos + Font.FontWidth)) ||
        (SSD1306_HEIGHT <= (SSD1306.ypos + Font.FontHeight))) {
        if (wrap) {
            if (SSD1306_WIDTH <= (SSD1306.xpos + Font.FontWidth)) {
                // Return to initial position shifted by font height
                SSD1306.xpos = SSD1306.xpos_init;
                SSD1306.ypos = (SSD1306.ypos_init + Font.FontHeight) * SSD1306.wrap_counter;
                SSD1306.wrap_counter++;
            } else if (SSD1306_HEIGHT <= (SSD1306.ypos + Font.FontHeight)) {
                return 1;
            }
        } else {
            // Not enough space on current line
            return 1;
        }
    }

    // Translate font to screenbuffer
    for (uint16_t i = 0; i < Font.FontHeight; i++) {
        pixel = Font.data[((ch - 32) * Font.FontHeight) + i];
        for (uint16_t j = 0; j < Font.FontWidth; j++) {
            if ((pixel << j) & 0x8000) {
                SSD1306_draw_pixel((SSD1306.xpos + j), (SSD1306.ypos + i), (SSD1306_COLOR)color);
            } else {
                SSD1306_draw_pixel((SSD1306.xpos + j), (SSD1306.ypos + i), (SSD1306_COLOR)!color);
            }
        }
    }

    // The current space is now taken
    SSD1306.xpos += Font.FontWidth;

    // Return written char for validation
    return ch;
}

/**
 * @brief           Write full string to screenbuffer
 * @param str       String to write to the screen
 * @param Font      Font struct with font parameters
 * @param color     Color to fill screen WHITE/BLACK
 * @param wrap      Used to check if the text needs to wrap
 * @return          Return char being written
*/
char SSD1306_writeString(const char* str, FontDef Font, SSD1306_COLOR color, uint8_t wrap)
{
    // Store initial cursor position
    SSD1306.xpos_init = SSD1306.xpos;
    SSD1306.ypos_init = SSD1306.ypos;
    SSD1306.wrap_counter = 1;

    // Write until null-byte
    while (*str) {
        if (!(SSD1306_write_char(*str, Font, color, wrap) != *str)) {
            // Char could not be written
            return *str;
        }

        // Next char
        str++;
    }

    return *str;
}

/**
 * @brief           Write image to screenbuffer
 *                  Each image section is 16-bits wide
 * @param Img       Image struct with image parameters
 * @param color     Color to fill screen WHITE/BLACK
*/
void SSD1306_writeImg(ImgDef Img, SSD1306_COLOR color)
{
    uint32_t pixel;

    // Store initial cursor position
    SSD1306.xpos_init = SSD1306.xpos;
    SSD1306.ypos_init = SSD1306.ypos;
    
    lastImg = Img;

    // Check remaining space on current line
    if ((SSD1306_WIDTH <= (SSD1306.xpos + Img.imgWidth)) ||
        (SSD1306_HEIGHT <= (SSD1306.ypos + Img.imgHeight))) {
        // Not enough space on current line
        return;
    }
    
    // Write each inidividual section of image
    for (uint8_t imgSection = 0; imgSection < Img.imgSections; imgSection++) {
        // Translate font to screenbuffer
        for (uint32_t i = 0; i < Img.imgHeight; i++) {
            pixel = Img.data[i + (imgSection * Img.imgHeight)];
            for (uint32_t j = 0; j < Img.imgWidth; j++) {
                if ((pixel << j) & 0x8000) {
                    SSD1306_draw_pixel((SSD1306.xpos + j), (SSD1306.ypos + i), (SSD1306_COLOR)color);
                } else {
                    SSD1306_draw_pixel((SSD1306.xpos + j), (SSD1306.ypos + i), (SSD1306_COLOR)!color);
                }
            }
        }
        
        // The current space is now taken
        SSD1306.xpos += Img.imgWidth;
    }
}
