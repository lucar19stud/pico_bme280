#include <stdio.h>
#include "pico/stdlib.h"
#include "bme280.h"

#define I2C_PORT  i2c0
#define I2C_SDA   4
#define I2C_SCL   5
#define I2C_BAUD  (100 * 1000)   // standard mode: easy to capture with logic analyzer

// Fault injection: 0 = normal operation
#define FAULT 0

int main()
{
    stdio_init_all();
    sleep_ms(5000);

    uint baud = i2c_init(I2C_PORT, I2C_BAUD);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);

    printf("i2C baud: %u Hz\n", baud);
    bme280_t bme = {.i2c = I2C_PORT, .addr = BME280_ADDR_LOW};

    int32_t adc_T;
    int32_t adc_P;
    int32_t adc_H;
    uint32_t seq = 0;
    bool calib_ok = false;

    while (true) {
        seq++;

        uint8_t id = 0;
        int err = bme280_check_chip_id(&bme, &id);
        if (err != BME280_OK) {
            printf("ERROR seq=%u err=%d id=0x%02X\n", (unsigned)seq, err, id);
            calib_ok = false;
            sleep_ms(1000);
            continue;
        }

        if (!calib_ok) {
            err = bme280_read_calib(&bme);
            if (err != BME280_OK) {
                printf("ERROR seq=%u err=%d (calib)\n", (unsigned)seq, err);
                sleep_ms(1000);
                continue;
            }
            calib_ok = true;
        }


        err = bme280_measure_raw(&bme, &adc_T, &adc_P, &adc_H);
        if (err != BME280_OK) {
            printf("ERROR seq=%u err=%d\n", (unsigned)seq, err);
            sleep_ms(1000);
            continue;
        }


        int32_t T = bme280_compensate_T(&bme, adc_T);
        if (FAULT == 3) T = T + 5000;
        uint32_t P = bme280_compensate_P(&bme, adc_P);
        uint32_t H = bme280_compensate_H(&bme, adc_H);

        uint32_t p_pa = P / 256;
        uint32_t h_cpc = H * 100 / 1024;

        printf("MEAS seq=%u id=0x%02X err=0 T=%d P=%u H=%u adcT=%d adcP=%d adcH=%d\n",
               (unsigned)seq, id, (int)T, (unsigned)p_pa, (unsigned)h_cpc,
               (int)adc_T, (int)adc_P, (int)adc_H);
        
        sleep_ms(1000);
    }
}


