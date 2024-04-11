#ifndef I2C_DRIVER_H
#define I2C_DRIVER_H

#include <stdint.h>

void I2C_init(void);
void I2C_start(void);
void I2C_stop(void);
uint8_t I2C_write(uint8_t data, uint32_t timeout);
uint8_t I2C_writeSlaveAddress(uint8_t address, uint32_t timeout);
uint8_t I2C_writeMulti(uint8_t *data, uint8_t size, uint32_t timeout);

#endif // I2C_DRIVER_H