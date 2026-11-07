#include <stdio.h>
#include "pico/stdlib.h"

#define BUTTON_PIN 15
#define LED_PIN 14
#define DEBOUNCE_MS 10
#define DEBOUNCE_MS_DUMMY (1000*60)

static volatile bool button_pressed = false;
static volatile uint32_t isr_counter = 0;

static int64_t debounce(alarm_id_t, void *user_data) {
    if (!gpio_get(BUTTON_PIN)) {
        button_pressed = true;
    }

    // activates irs for the next press
    gpio_set_irq_enabled(BUTTON_PIN, GPIO_IRQ_EDGE_FALL, true);
    // alarm only once. 
    return 0;
}

static void button_isr(uint gpio, uint32_t events) {
    // Button pin 15 AND falling edge
    if (gpio == BUTTON_PIN && (events & GPIO_IRQ_EDGE_FALL)) {
        isr_counter++;

        // deactivates isr for pin 15
        gpio_set_irq_enabled(BUTTON_PIN, GPIO_IRQ_EDGE_FALL, false);
        
        // calls `debounce` after alarm 
        if (add_alarm_in_ms(DEBOUNCE_MS, debounce, NULL, true) < 0) {
            // No alarm slot: re-enable irs so the button never goes dead
            gpio_set_irq_enabled(BUTTON_PIN, GPIO_IRQ_EDGE_FALL, true);
        }
        
    }
}


// static int64_t dummy(alarm_id_t, void *user_data) {
//     return 0;
// }

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

    // sleep_ms(2000);
    // int n = 0;
    // while (add_alarm_in_ms(DEBOUNCE_MS_DUMMY, dummy, NULL, true) > 0) {
    //     n++;
    // }
    // sleep_ms(5000);
    // printf("alarm pool full after %d dummy alarms\n", n);

    while (true) {
        if(button_pressed) {
            button_pressed = false;
            presses++;
            led_on = !led_on;
            gpio_put(LED_PIN, led_on);
            printf("press %lu, raw edges %lu\n", (unsigned long)presses, (unsigned long)isr_counter);
        }
    }
}
