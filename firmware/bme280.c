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

int bme280_write_reg(const bme280_t *dev, uint8_t reg, uint8_t value) {
    uint8_t buf[2];
    buf[0] = reg;
    buf[1] = value;
    int ret = i2c_write_timeout_us(dev->i2c, dev->addr, buf, 2, false, BME280_I2C_TIMEOUT_US);
    if (ret < 0) {
        if (ret == PICO_ERROR_TIMEOUT) return BME280_ERR_TIMEOUT;
        return BME280_ERR_NACK;
    }
    if (ret != 2) return BME280_ERR_NACK;
    return BME280_OK;
}

int bme280_measure_raw(const bme280_t *dev, int32_t *adc_T, int32_t *adc_P, int32_t *adc_H){
    int err = bme280_write_reg(dev, 0xF2, 0x01);
    if (err != BME280_OK) return err;
    err = bme280_write_reg(dev, 0xF4, 0x25); //001 001 01 = 0x25
    if (err != BME280_OK) return err;

    sleep_ms(10);
    
    uint8_t out;
    for (int i = 0; i < 21; i++) {
        err = bme280_read_regs(dev, 0xF3, &out, 1);
        if (err != BME280_OK) {
            printf("measure err: %d\n", err);
            sleep_ms(1000);
            continue;   
        }

        if ((out & 0x08) == 0) {
            break;
        } 
        if (i == 20) return BME280_ERR_TIMEOUT;
        sleep_ms(1);
    }
    
    uint8_t raw[8];
    err = bme280_read_regs(dev, 0xF7, raw, 8);
    if (err != BME280_OK) return err;

    *adc_P = (int32_t) (raw[0] << 12) | (raw[1] << 4) | (raw[2] >> 4);
    *adc_T = (int32_t) (raw[3] << 12) | (raw[4] << 4) | (raw[5] >> 4);
    *adc_H = (int32_t) (raw[6] << 8) | raw[7];
    return BME280_OK;
}

int32_t bme280_compensate_T(bme280_t *dev, int32_t adc_T) {
    int32_t T1 = dev->calib.T1;
    int32_t T2 = dev->calib.T2;
    int32_t T3 = dev->calib.T3;

    int32_t a    = (adc_T >> 3) - (T1 << 1);
    int32_t var1 = (a * T2) >> 11;

    int32_t b    = (adc_T >> 4) - T1;
    int32_t var2 = (((b * b) >> 12) * T3) >> 14;

    dev->t_fine = var1 + var2;

    int32_t T = (dev->t_fine * 5 + 128) >> 8;
    return T;
}

int32_t bme280_compensate_P(bme280_t *dev, int32_t adc_P) {
    int64_t var1 = dev->t_fine - 128000;
    int64_t var2 = var1 * var1 * dev->calib.P6;
    var2 = var2 + ((var1 * dev->calib.P5)<<17);
    var2 = var2 + ((int64_t)dev->calib.P4<<35);
    var1 = ((var1 * var1 * dev->calib.P3)>>8) + ((var1 * dev->calib.P2)<<12);
    var1 = ((((int64_t)1<<47) + var1)*(dev->calib.P1))>>33;
    if (var1 == 0) return 0;

    int64_t P = 1048576 - adc_P;
    P = (((P<<31)-var2)*3125)/var1;
    var1 = ((dev->calib.P9) * (P>>13) * (P>>13)) >> 25; 
    var2 = (dev->calib.P8 * P) >> 19;
    P = ((P + var1 + var2) >> 8) + ((dev->calib.P7) << 4);
    return (uint32_t) P;
}

int32_t bme280_compensate_H(bme280_t *dev, int32_t adc_H) {
    int32_t H1 = dev->calib.H1;
    int32_t H2 = dev->calib.H2;
    int32_t H3 = dev->calib.H3;
    int32_t H4 = dev->calib.H4;
    int32_t H5 = dev->calib.H5;
    int32_t H6 = dev->calib.H6;

    int32_t x = dev->t_fine - 76800;
    int32_t f = (adc_H << 14) - (H4 << 20) - (H5 * x);
    f = (f + 16384) >> 15;
    int32_t g1 = (x * H6) >> 10;
    int32_t g2 = ((x * H3) >> 11) + 32768;
    int32_t g  = (g1 * g2) >> 10;
    g = g + 2097152;
    g = (g * H2 + 8192) >> 14;
    x = f * g;
    int32_t c = (x >> 15) * (x >> 15);
    c = (c >> 7) * H1;
    c = c >> 4;
    x = x - c;
    if (x < 0)         x = 0;
    if (x > 419430400) x = 419430400;

    return (uint32_t)(x >> 12);
}