#ifndef SSD1306_IMGS_H
#define SSD1306_IMGS_H

#include <stdint.h>

// Image struct
typedef struct {
    uint8_t imgWidth;     // Font width in pixels
    uint8_t imgHeight;    // Font height in pixels
    uint16_t *data;       // Pointer to data font data array
    uint8_t imgSections;  // Number of sections in image
} ImgDef;

extern ImgDef Ryu_32x36;
extern ImgDef DogDown_22x20;
extern ImgDef DogUp_22x20;

#endif // SSD1306_IMGS_H
