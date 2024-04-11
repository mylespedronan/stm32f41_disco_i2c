#ifndef SSD1306_DRIVER_H
#define SSD1306_DRIVER_H

#include <stdint.h>
#include "ssd1306_fonts.h"
#include "ssd1306_imgs.h"

typedef enum {
    BLACK = 0x00,               // Black color, no pixel
    WHITE = 0x01,               // Pixel is set. Color depends on LCD
} SSD1306_COLOR;

typedef struct {
    uint16_t xpos;              // Current x position
    uint16_t ypos;              // Current y position
    uint16_t xpos_init;         // Initial x position of image/font
    uint16_t ypos_init;         // Initial y position of image/font
    uint8_t wrap_counter;       // Amount of times image/font wrapped around screen buffer
} SSD1306_t;

uint8_t SSD1306_init(void);
uint8_t SSD1306_update(void);
void SSD1306_fill(SSD1306_COLOR color);
void SSD1306_setCursor(uint8_t x, uint8_t y);

char SSD1306_writeString(const char* str, FontDef Font, SSD1306_COLOR color, uint8_t wrap);
void SSD1306_writeImg(ImgDef Img, SSD1306_COLOR color);

void SSD1306_moveImageRight(void);
void SSD1306_moveImageLeft(void);

#endif // SSD1306_DRIVER_H
