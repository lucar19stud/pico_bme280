#include <stdio.h>
#include "pico/stdlib.h"

#define BUTTON_PIN 15
#define LED_PIN 14

static volatile bool button_pressed = false;
static volatile uint32_t bounce_count = 0;

static void button_isr(uint gpio, uint32_t events) {
    if (gpio == BUTTON_PIN && (events & GPIO_IRQ_EDGE_FALL)) {
        button_pressed = true;
        bounce_count++;
    }
}

int main()
{
    stdio_init_all();

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);

    gpio_set_irq_enabled_with_callback(BUTTON_PIN, GPIO_IRQ_EDGE_FALL, true, &button_isr);

    uint32_t presses = 0;
    bool led_on = false;

    while (true) {
        if(button_pressed) {
            button_pressed = false;
            presses++;
            led_on = !led_on;
            gpio_put(LED_PIN, led_on);
            printf("press %lu, raw edges %lu\n", (unsigned long)presses, (unsigned long)bounce_count);
        }
    }
}
