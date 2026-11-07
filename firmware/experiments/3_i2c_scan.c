#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

#define I2C_PORT  i2c0
#define I2C_SDA   4
#define I2C_SCL   5
#define I2C_BAUD  (100 * 1000)   // standard mode: easy to capture at 24 MHz

// 0x00-0x07 and 0x78-0x7F are reserved by the I2C specification
static bool reserved_addr(uint8_t addr) {
    return (addr & 0x78) == 0 || (addr & 0x78) == 0x78;
}

int main()
{
    stdio_init_all();
    sleep_ms(5000);

    i2c_init(I2C_PORT, I2C_BAUD);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);

    while (true) {
        printf("I2C scan:\n");
        for (uint8_t addr = 0; addr < 0x80; addr++) {
            if (reserved_addr(addr)) continue;
            uint8_t dummy;
            
            // Device must ACK its address byte; NACK -> negative return
            int ret = i2c_read_blocking(I2C_PORT, addr, &dummy, 1, false);
            if (ret >= 0) printf("  device at 0x%02X\n", addr);
        }
        sleep_ms(3000);
    }
}
