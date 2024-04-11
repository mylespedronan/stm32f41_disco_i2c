#ifndef SSD1306_FONTS_H
#define SSD1306_FONTS_H

#include <stdint.h>

// Font struct
typedef struct {
    const uint8_t FontWidth;    // Font width in pixels
    const uint8_t FontHeight;   // Font height in pixels
    const uint16_t *data;       // Pointer to data font data array
} FontDef;

extern FontDef Font_7x10;
extern FontDef Font_11x18;
extern FontDef Font_16x26;

#endif // SSD1306_FONTS_H
