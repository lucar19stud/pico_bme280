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
    uint8_t id = 0;
    bme280_t bme = {.i2c = I2C_PORT, .addr = BME280_ADDR_LOW};
    int err = bme280_check_chip_id(&bme, &id);
    printf("chip id: 0x%02X  err: %d\n", id, err);

    if (err != BME280_OK) {
        while (true) {
            printf("ERROR: sensor init failed (chip id 0x%02X, err %d)\n", id, err);
            sleep_ms(1000);
        }
    }
    err = bme280_read_calib(&bme);
    printf("read_calib err: %d\n", err);

    int32_t adc_T;
    int32_t adc_P;
    int32_t adc_H;
    int32_t T;
    int32_t P;
    int32_t H;
    while (true) {
        err = bme280_measure_raw(&bme, &adc_T, &adc_P, &adc_H);
        printf("calib measure err: %d\n", err);

        T = bme280_compensate_T(&bme, adc_T);
        printf("T = %d.%02d C\n", T / 100, T % 100);

        P = bme280_compensate_P(&bme, adc_P);
        printf("P = %d.%02d hPa\n", (int)(P/256)/100, (int)(P/256)%100);

        H = bme280_compensate_H(&bme, adc_H);
        printf("H = %d.%02d %%\n", (int)(H / 1024), (int)((H % 1024) * 100 / 1024));
        sleep_ms(1000);
    }
}


