#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/gpio.h"
#include "pico/bootrom.h"

#include "ws2812.pio.h"

// Define os pinos do LED RGB
#define LED_GREEN 11
#define LED_BLUE 12
#define LED_RED 13

// Define o pino da matriz de LEDs
#define LED_PIN 7
#define NUM_LEDS 25

// Define os pinos dos botões
#define BUTTON_A 5
#define BUTTON_B 6

int main() {
    stdio_init_all();

    // PIO ws2812ing example
    PIO pio = pio0;
    uint offset = pio_add_program(pio, &ws2812_program);

    while (true) {
        printf("Hello, world!\n");
        sleep_ms(1000);
    }
}
