#include <pico/stdlib.h>

#include "diff_drive.h"

const uint LED_PIN = PICO_DEFAULT_LED_PIN;

void blink_led(int times, int delay_ms) {
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    for (int i = 0; i < times; i++) {
        gpio_put(LED_PIN, 1); // Turn LED on
        sleep_ms(delay_ms);
        gpio_put(LED_PIN, 0); // Turn LED off
        sleep_ms(delay_ms);
    }
}
