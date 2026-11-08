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

    int err = bme280_read_calib(&bme);
    printf("read_calib err: %d\n", err);
    
    printf("T1 = %u\n", bme.calib.T1);
    printf("T2 = %d\n", bme.calib.T2);
    printf("T3 = %d\n", bme.calib.T3);

    printf("P1 = %u\n", bme.calib.P1);
    printf("P2 = %d\n", bme.calib.P2);
    printf("P3 = %d\n", bme.calib.P3);
    printf("P4 = %d\n", bme.calib.P4);
    printf("P5 = %d\n", bme.calib.P5);
    printf("P6 = %d\n", bme.calib.P6);
    printf("P7 = %d\n", bme.calib.P7);
    printf("P8 = %d\n", bme.calib.P8);
    printf("P9 = %d\n", bme.calib.P9);

    printf("H1 = %u\n", bme.calib.H1);
    printf("H2 = %d\n", bme.calib.H2);
    printf("H3 = %u\n", bme.calib.H3);
    printf("H4 = %d\n", bme.calib.H4);
    printf("H5 = %d\n", bme.calib.H5);
    printf("H6 = %d\n", bme.calib.H6);

    int b[26];
    while (true) {

        int err = bme280_read_regs(&bme, 0x88, b, 26);
        printf("calib read err: %d\n", err);
        for (int i = 0; i < 26; i++) {
            printf("reg 0x%02X = 0x%02X  (%3u)\n", 0x88 + i, b[i], b[i]);
        }
        sleep_ms(1000);
    }
}
