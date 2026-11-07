#include "bme280.h"
#include "pico/error.h"

#define BME280_I2C_TIMEOUT_US  10000

int bme280_read_regs(const bme280_t *dev, uint8_t reg, uint8_t *buf, size_t len) {
    int ret = i2c_write_timeout_us(dev->i2c, dev->addr, &reg, 1, true, BME280_I2C_TIMEOUT_US);
    if (ret < 0) {
        if (ret == PICO_ERROR_TIMEOUT) return BME280_ERR_TIMEOUT;
        return BME280_ERR_NACK;
    }
    if (ret != 1) return BME280_ERR_NACK;

    ret = i2c_read_timeout_us(dev->i2c, dev->addr, buf, len,false, BME280_I2C_TIMEOUT_US);
    if (ret < 0) {
        if (ret == PICO_ERROR_TIMEOUT) return BME280_ERR_TIMEOUT;
        return BME280_ERR_NACK;
    }
    if (ret != (int)len) return BME280_ERR_NACK;

    return BME280_OK;
}

int bme280_check_chip_id(const bme280_t *dev, uint8_t *id_out) {
    uint8_t id;
    int err = bme280_read_regs(dev, BME280_REG_CHIP_ID, &id, 1);
    if (err != BME280_OK) return err;

    // Report the value even on mismatch: "wrong chip" is more useful
    // to the tester than just "error"
    if (id_out) *id_out = id;
    return (id == BME280_CHIP_ID) ? BME280_OK : BME280_ERR_CHIP_ID;
}
