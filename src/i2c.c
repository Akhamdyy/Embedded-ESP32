/******************************************************************************
 *
 * Module: I2C
 *
 * File Name: i2c.c
 *
 * Description: Source file for the ESP32 I2C master driver (MCAL layer).
 *              Wraps driver/i2c.h. All ESP-IDF I2C calls are confined here.
 *
 *******************************************************************************/

#include "i2c.h"
#include "driver/i2c.h"

/*******************************************************************************
 *                         Private Definitions                                 *
 *******************************************************************************/

#define I2C_PORT        I2C_NUM_0
#define I2C_SDA_GPIO    27
#define I2C_SCL_GPIO    33
#define I2C_FREQ_HZ     400000
#define I2C_TIMEOUT_MS  50

/*******************************************************************************
 *                          Functions Definitions                              *
 *******************************************************************************/

/*
 * Description :
 * Initialize I2C_NUM_0 as master on GPIO 27 (SDA) / GPIO 33 (SCL) at 400 kHz.
 */
void I2C_init(void)
{
    i2c_config_t conf = {
        .mode             = I2C_MODE_MASTER,
        .sda_io_num       = I2C_SDA_GPIO,
        .scl_io_num       = I2C_SCL_GPIO,
        .sda_pullup_en    = GPIO_PULLUP_ENABLE,
        .scl_pullup_en    = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_FREQ_HZ,
    };
    i2c_param_config(I2C_PORT, &conf);
    i2c_driver_install(I2C_PORT, I2C_MODE_MASTER, 0, 0, 0);
}

/*
 * Description :
 * Write one byte to a register on the device at the given 7-bit address.
 */
boolean I2C_writeReg(uint8 dev_addr, uint8 reg, uint8 val)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (dev_addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_write_byte(cmd, val, true);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_PORT, cmd, pdMS_TO_TICKS(I2C_TIMEOUT_MS));
    i2c_cmd_link_delete(cmd);
    return (ret == ESP_OK) ? TRUE : FALSE;
}

/*
 * Description :
 * Read 'len' bytes starting at 'reg' from the device at 'dev_addr'.
 */
boolean I2C_readBytes(uint8 dev_addr, uint8 reg, uint8 *buf, uint8 len)
{
    if (buf == NULL_PTR || len == 0)
    {
        return FALSE;
    }

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (dev_addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (dev_addr << 1) | I2C_MASTER_READ, true);
    i2c_master_read(cmd, buf, len, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_PORT, cmd, pdMS_TO_TICKS(I2C_TIMEOUT_MS));
    i2c_cmd_link_delete(cmd);
    return (ret == ESP_OK) ? TRUE : FALSE;
}
