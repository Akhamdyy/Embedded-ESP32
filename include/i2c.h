/******************************************************************************
 *
 * Module: I2C
 *
 * File Name: i2c.h
 *
 * Description: Header file for the ESP32 I2C master driver (MCAL layer).
 *              Wraps driver/i2c.h and confines all ESP-IDF I2C details to
 *              i2c.c. Exposes a simple register-oriented API for HAL modules.
 *
 * Wiring (fixed to I2C_NUM_0):
 *   SDA -> GPIO 27  (PORTC PIN1)
 *   SCL -> GPIO 33  (PORTC PIN3)
 *
 *******************************************************************************/

#ifndef I2C_H_
#define I2C_H_

#include "std_types.h"

/*******************************************************************************
 *                              Functions Prototypes                           *
 *******************************************************************************/

/*
 * Description :
 * Initialize I2C_NUM_0 as master on GPIO 27 (SDA) / GPIO 33 (SCL) at 400 kHz.
 * Must be called once before any read/write operation.
 */
void I2C_init(void);

/*
 * Description :
 * Write one byte to a register on the device at the given 7-bit address.
 * Returns TRUE on success, FALSE on timeout or NACK.
 */
boolean I2C_writeReg(uint8 dev_addr, uint8 reg, uint8 val);

/*
 * Description :
 * Read 'len' bytes starting at 'reg' from the device at 'dev_addr'.
 * Bytes are stored into buf[0..len-1].
 * Returns TRUE on success, FALSE on timeout or NACK.
 */
boolean I2C_readBytes(uint8 dev_addr, uint8 reg, uint8 *buf, uint8 len);

#endif /* I2C_H_ */
