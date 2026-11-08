#include <stdio.h>
#include "pico/stdlib.h"
#include "bme280.h"

#define I2C_PORT  i2c0
#define I2C_SDA   4
#define I2C_SCL   5
#define I2C_BAUD  (100 * 1000)   // standard mode: easy to capture with logic analyzer

int main()
{
    stdio_init_all();
    sleep_ms(5000);

    uint baud = i2c_init(I2C_PORT, I2C_BAUD);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    printf("i2C baud: %u Hz\n", baud);

    bme280_t bme = {.i2c = I2C_PORT, .addr = BME280_ADDR_LOW};

    uint8_t b[26];
    
    while (true) {
        //uint8_t id = 0;
        //int err = bme280_check_chip_id(&bme, &id);
        //printf("chip id: 0x%02X  err: %d\n", id, err);
        sleep_ms(1000);
        int err = bme280_read_regs(&bme, 0x88, b, 26);
        //if (err == BME280_ERR_TIMEOUT) {
        //    i2c_deinit(I2C_PORT);
        //    i2c_init(I2C_PORT, I2C_BAUD);
        //}
        printf("calib read err: %d\n", err);
        //for (int i = 0; i < 26; i++) {
        //    printf("reg 0x%02X = 0x%02X  (%3u)\n", 0x88 + i, b[i], b[i]);
        //}
    }
}
