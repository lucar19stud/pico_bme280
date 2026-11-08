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

int bme280_read_calib(bme280_t *dev) {
    uint8_t b[26];
    uint8_t h[7];
    int err;

    err = bme280_read_regs(dev, BME280_REG_CALIB_TP, b, 26);
    if (err != BME280_OK) return err;

    err = bme280_read_regs(dev, BME280_REG_CALIB_H, h, 7);
    if (err != BME280_OK) return err;

    dev->calib.T1 = (uint16_t) ((b[1] << 8) | b[0]);
    dev->calib.T2 = (int16_t) ((b[3] << 8) | b[2]);
    dev->calib.T3 = (int16_t) ((b[5] << 8) | b[4]);

    dev->calib.P1 = (uint16_t) ((b[7] << 8) | b[6]);
    dev->calib.P2 = (int16_t) ((b[9] << 8) | b[8]);
    dev->calib.P3 = (int16_t) ((b[11] << 8) | b[10]);
    dev->calib.P4 = (int16_t) ((b[13] << 8) | b[12]);
    dev->calib.P5 = (int16_t) ((b[15] << 8) | b[14]);
    dev->calib.P6 = (int16_t) ((b[17] << 8) | b[16]);
    dev->calib.P7 = (int16_t) ((b[19] << 8) | b[18]);
    dev->calib.P8 = (int16_t) ((b[21] << 8) | b[20]);
    dev->calib.P9 = (int16_t) ((b[23] << 8) | b[22]);

    dev->calib.H1 = (uint8_t) b[25];
    dev->calib.H2 = (int16_t) ((h[1] << 8) | h[0]);
    dev->calib.H3 = (uint8_t) h[2];

    int8_t  h4_upper = (int8_t)h[3];   // int8_t keeps the sign bit
    uint8_t h4_lower = h[4] & 0x0F;    // low nibble of 0xE5
    dev->calib.H4 = h4_upper * 16 + h4_lower;

    int8_t  h5_upper = (int8_t)h[5];
    uint8_t h5_lower = h[4] >> 4;      // high nibble of 0xE5
    dev->calib.H5 = h5_upper * 16 + h5_lower;

    dev->calib.H6 = (int8_t) h[6];

    return BME280_OK;
}