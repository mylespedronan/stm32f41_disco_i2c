#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

uint8_t SysClockConfig(void);
uint8_t TIM2init(void);
void Delay_us(uint16_t us);
void Delay_ms(uint16_t ms);

#endif // TIMER_H