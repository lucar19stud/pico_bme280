#ifndef BME280_H
#define BME280_H

#include <stdint.h>
#include <stddef.h>
#include "hardware/i2c.h"

#define BME280_ADDR_LOW   0x76   // ADDR pin to GND
#define BME280_ADDR_HIGH  0x77   // ADDR pin to 3V3
#define BME280_CHIP_ID    0x60

#define BME280_REG_CHIP_ID   0xD0

// Driver error codes: 0 = OK, negative = error (never fail silently)
enum {
    BME280_OK          =  0,
    BME280_ERR_NACK    = -1,  // device did not acknowledge (wrong address, not connected)
    BME280_ERR_TIMEOUT = -2,  // bus stuck or transfer took too long
    BME280_ERR_CHIP_ID = -3,  // device answered, but is not a BME280
};

// Device handle: which bus, which address
typedef struct {
    i2c_inst_t *i2c;
    uint8_t addr;
} bme280_t;

int bme280_read_regs(const bme280_t *dev, uint8_t reg, uint8_t *buf, size_t len);
int bme280_check_chip_id(const bme280_t *dev, uint8_t * id_out);

#endif // BME280_H