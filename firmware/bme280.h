#ifndef BME280_H
#define BME280_H

#include <stdint.h>
#include <stddef.h>
#include "hardware/i2c.h"

#define BME280_ADDR_LOW   0x76   // ADDR pin to GND
#define BME280_ADDR_HIGH  0x77   // ADDR pin to 3V3
#define BME280_CHIP_ID    0x60

#define BME280_REG_CHIP_ID   0xD0

#define BME280_REG_CALIB_TP  0x88   // 0x88..0xA1: T1..T3, P1..P9, (0xA0 unused), H1
#define BME280_REG_CALIB_H   0xE1   // 0xE1..0xE7: H2..H6

// Driver error codes: 0 = OK, negative = error (never fail silently)
enum {
    BME280_OK          =  0,
    BME280_ERR_NACK    = -1,  // device did not acknowledge (wrong address, not connected)
    BME280_ERR_TIMEOUT = -2,  // bus stuck or transfer took too long
    BME280_ERR_CHIP_ID = -3,  // device answered, but is not a BME280
};

typedef struct {
    uint16_t T1; int16_t T2, T3;
    uint16_t P1; int16_t P2, P3, P4, P5, P6, P7, P8, P9;
    uint8_t  H1; int16_t H2; uint8_t H3; int16_t H4, H5; int8_t H6;
} bme280_calib_t;

// Device handle: which bus, which address
typedef struct {
    i2c_inst_t *i2c;
    uint8_t addr;
    bme280_calib_t calib;
    int32_t t_fine;
} bme280_t;

int bme280_read_regs(const bme280_t *dev, uint8_t reg, uint8_t *buf, size_t len);
int bme280_check_chip_id(const bme280_t *dev, uint8_t * id_out);
int bme280_read_calib(bme280_t *dev);
int bme280_write_reg(const bme280_t *dev, uint8_t reg, uint8_t value);
int bme280_measure_raw(const bme280_t *dev, int32_t *adc_T, int32_t *adc_P, int32_t *adc_H);
int32_t bme280_compensate_T(bme280_t *dev, int32_t adc_T);
int32_t bme280_compensate_P(bme280_t *dev, int32_t adc_P);
int32_t bme280_compensate_H(bme280_t *dev, int32_t adc_H);



#endif // BME280_H