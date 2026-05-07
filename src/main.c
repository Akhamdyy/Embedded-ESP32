#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
// #include "driver/i2c.h"
// #include "esp_log.h"
#include "bluetooth.h"

/* ── MPU-6050 / I2C pin assignments ── commented out for BT-only test ──── */
#if 0
#define I2C_MASTER_SDA_IO       27
#define I2C_MASTER_SCL_IO       33
#define I2C_MASTER_NUM          I2C_NUM_0
#define I2C_MASTER_FREQ_HZ      100000
#define I2C_MASTER_TIMEOUT_MS   1000

#define MPU6050_ADDR            0x68

#define REG_WHO_AM_I            0x75
#define REG_PWR_MGMT_1          0x6B
#define REG_SMPLRT_DIV          0x19
#define REG_CONFIG              0x1A
#define REG_GYRO_CONFIG         0x1B
#define REG_ACCEL_CONFIG        0x1C
#define REG_ACCEL_XOUT_H        0x3B
#define REG_TEMP_OUT_H          0x41
#define REG_GYRO_XOUT_H         0x43

#define ACCEL_SENSITIVITY       16384.0f
#define GYRO_SENSITIVITY        131.0f

static const char *TAG = "MPU6050_TEST";
#endif /* MPU-6050 definitions */

/* ── MPU-6050 low-level I2C helpers ── commented out for BT-only test ───── */
#if 0
static esp_err_t mpu_write_reg(uint8_t reg, uint8_t data)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (MPU6050_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg,  true);
    i2c_master_write_byte(cmd, data, true);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd,
                        pdMS_TO_TICKS(I2C_MASTER_TIMEOUT_MS));
    i2c_cmd_link_delete(cmd);
    return ret;
}

static esp_err_t mpu_read_regs(uint8_t reg, uint8_t *buf, size_t len)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (MPU6050_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (MPU6050_ADDR << 1) | I2C_MASTER_READ, true);
    if (len > 1)
        i2c_master_read(cmd, buf, len - 1, I2C_MASTER_ACK);
    i2c_master_read_byte(cmd, buf + len - 1, I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd,
                        pdMS_TO_TICKS(I2C_MASTER_TIMEOUT_MS));
    i2c_cmd_link_delete(cmd);
    return ret;
}

static inline int16_t combine(uint8_t msb, uint8_t lsb)
{
    return (int16_t)(((uint16_t)msb << 8) | lsb);
}
#endif /* MPU-6050 I2C helpers */

/* ── I2C bus scanner ── commented out for BT-only test ─────────────────── */
#if 0
static void i2c_scan(void)
{
    printf("\n");
    printf("┌─────────────────────────────────────────────┐\n");
    printf("│           I2C Bus Scan (0x01 – 0x7E)        │\n");
    printf("└─────────────────────────────────────────────┘\n");

    int found = 0;
    for (uint8_t addr = 0x01; addr < 0x7F; addr++) {
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
        i2c_master_stop(cmd);
        esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd,
                            pdMS_TO_TICKS(50));
        i2c_cmd_link_delete(cmd);

        if (ret == ESP_OK) {
            printf("  [FOUND] Device at address: 0x%02X", addr);
            if (addr == 0x68) printf("  <- MPU-6050 (AD0=GND)  OK");
            if (addr == 0x69) printf("  <- MPU-6050 (AD0=3.3V) OK");
            printf("\n");
            found++;
        }
    }

    if (found == 0) {
        printf("  [ERROR] No devices found!\n");
        printf("  Check: VCC=3.3V, GND, SDA=GPIO27, SCL=GPIO33, AD0=GND\n");
    } else {
        printf("  Total devices found: %d\n", found);
    }
    printf("\n");
}
#endif /* i2c_scan */

/* ── MPU-6050 init ── commented out for BT-only test ───────────────────── */
#if 0
static esp_err_t mpu6050_init(void)
{
    esp_err_t ret;

    uint8_t who_am_i = 0;
    ret = mpu_read_regs(REG_WHO_AM_I, &who_am_i, 1);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "WHO_AM_I read FAILED (I2C error). Check wiring!");
        return ret;
    }

    printf("  WHO_AM_I register = 0x%02X", who_am_i);
    if (who_am_i == 0x68) {
        printf("  OK  (expected 0x68 -- MPU-6050 confirmed)\n");
    } else {
        printf("  FAIL  (expected 0x68 -- wrong chip or wiring issue)\n");
        return ESP_FAIL;
    }

    ret = mpu_write_reg(REG_PWR_MGMT_1, 0x00);
    if (ret != ESP_OK) { ESP_LOGE(TAG, "Wake-up write failed"); return ret; }
    printf("  PWR_MGMT_1 = 0x00  OK  (chip woken from sleep)\n");
    vTaskDelay(pdMS_TO_TICKS(100));

    ret = mpu_write_reg(REG_SMPLRT_DIV, 0x07);
    if (ret != ESP_OK) { ESP_LOGE(TAG, "SMPLRT_DIV write failed"); return ret; }

    ret = mpu_write_reg(REG_CONFIG, 0x05);
    if (ret != ESP_OK) { ESP_LOGE(TAG, "CONFIG write failed"); return ret; }

    ret = mpu_write_reg(REG_GYRO_CONFIG, 0x00);
    if (ret != ESP_OK) { ESP_LOGE(TAG, "GYRO_CONFIG write failed"); return ret; }

    ret = mpu_write_reg(REG_ACCEL_CONFIG, 0x00);
    if (ret != ESP_OK) { ESP_LOGE(TAG, "ACCEL_CONFIG write failed"); return ret; }

    printf("  Configuration complete OK\n\n");
    return ESP_OK;
}
#endif /* mpu6050_init */

/* ── MPU-6050 read + BT send ── commented out for BT-only test ─────────── */
#if 0
static void mpu6050_read_and_print(void)
{
    uint8_t buf[14];

    esp_err_t ret = mpu_read_regs(REG_ACCEL_XOUT_H, buf, 14);
    if (ret != ESP_OK) {
        printf("  [ERROR] Read failed -- check connections\n");
        return;
    }

    int16_t raw_ax = combine(buf[0],  buf[1]);
    int16_t raw_ay = combine(buf[2],  buf[3]);
    int16_t raw_az = combine(buf[4],  buf[5]);
    int16_t raw_t  = combine(buf[6],  buf[7]);
    int16_t raw_gx = combine(buf[8],  buf[9]);
    int16_t raw_gy = combine(buf[10], buf[11]);
    int16_t raw_gz = combine(buf[12], buf[13]);

    float ax   = raw_ax / ACCEL_SENSITIVITY;
    float ay   = raw_ay / ACCEL_SENSITIVITY;
    float az   = raw_az / ACCEL_SENSITIVITY;
    float gx   = raw_gx / GYRO_SENSITIVITY;
    float gy   = raw_gy / GYRO_SENSITIVITY;
    float gz   = raw_gz / GYRO_SENSITIVITY;
    float temp = (raw_t / 340.0f) + 36.53f;

    const char *dir;
    if (gz > 15.0f)       dir = "TURN RIGHT";
    else if (gz < -15.0f) dir = "TURN LEFT";
    else                   dir = "STRAIGHT";
    printf("  [BT] Direction: %s\n", dir);
    bluetooth_send(dir);
    bluetooth_send("\r\n");

    printf("+---------------------------------------------------------+\n");
    printf("|  ACCELEROMETER (+-2 g)         RAW        CONVERTED     |\n");
    printf("+---------------------------------------------------------+\n");
    printf("|  Accel X                    %7d        %+7.3f g     |\n", raw_ax, ax);
    printf("|  Accel Y                    %7d        %+7.3f g     |\n", raw_ay, ay);
    printf("|  Accel Z                    %7d        %+7.3f g     |\n", raw_az, az);
    printf("+---------------------------------------------------------+\n");
    printf("|  GYROSCOPE (+-250 deg/s)       RAW        CONVERTED     |\n");
    printf("+---------------------------------------------------------+\n");
    printf("|  Gyro X  (roll rate)        %7d        %+7.2f d/s  |\n", raw_gx, gx);
    printf("|  Gyro Y  (pitch rate)       %7d        %+7.2f d/s  |\n", raw_gy, gy);
    printf("|  Gyro Z  (yaw rate) *       %7d        %+7.2f d/s  |\n", raw_gz, gz);
    printf("+---------------------------------------------------------+\n");
    printf("|  TEMPERATURE                   RAW        CONVERTED     |\n");
    printf("+---------------------------------------------------------+\n");
    printf("|  Chip temperature           %7d        %+7.2f C    |\n", raw_t, temp);
    printf("+---------------------------------------------------------+\n");
    printf("  * Gyro Z = yaw rate used by FSM for turn detection\n\n");
}
#endif /* mpu6050_read_and_print */

/* ═══════════════════════════════════════════════════════════════════════════
 *  APP MAIN  —  Bluetooth connectivity test
 * ═════════════════════════════════════════════════════════════════════════ */
void app_main(void)
{
    printf("\n");
    printf("+----------------------------------------------------------+\n");
    printf("|          Bluetooth Connectivity Test — WallRobot         |\n");
    printf("+----------------------------------------------------------+\n\n");

    if (bluetooth_init("WallRobot") == FALSE) {
        printf("[WARN] Bluetooth init failed — continuing without BT\n\n");
    }

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
