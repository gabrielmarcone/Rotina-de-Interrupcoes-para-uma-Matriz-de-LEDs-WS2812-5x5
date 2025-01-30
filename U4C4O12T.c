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

// Variáveis globais para PIO
static PIO  pio = pio0;
static uint sm  = 0;

// Variável global para controle do número exibido
volatile int current_number = 0;

// Debounce time
#define DEBOUNCE_TIME 200  // 200 ms
absolute_time_t last_press_time_A;
absolute_time_t last_press_time_B;

// Tabela de padrões para exibição de números na matriz WS2812
const uint32_t number_patterns[10][NUM_LEDS] = {
    // Número 0
    {
        0x000000, 0x000000, 0xFFFFFF, 0x000000, 0x000000, 
        0x000000, 0xFFFFFF, 0x000000, 0xFFFFFF, 0x000000, 
        0x000000, 0xFFFFFF, 0x000000, 0xFFFFFF, 0x000000, 
        0x000000, 0xFFFFFF, 0x000000, 0xFFFFFF, 0x000000, 
        0x000000, 0x000000, 0xFFFFFF, 0x000000, 0x000000
     },
    
    // Número 1
    {
        0x000000, 0x000000, 0xFFFFFF, 0x000000, 0x000000, 
        0x000000, 0x000000, 0xFFFFFF, 0x000000, 0x000000, 
        0x000000, 0x000000, 0xFFFFFF, 0x000000, 0x000000, 
        0x000000, 0xFFFFFF, 0xFFFFFF, 0x000000, 0x000000, 
        0x000000, 0x000000, 0xFFFFFF, 0x000000, 0x000000
    },

    // Número 2
    {
        0x000000, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0x000000, 
        0x000000, 0xFFFFFF, 0x000000, 0x000000, 0x000000, 
        0x000000, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0x000000, 
        0x000000, 0x000000, 0x000000, 0xFFFFFF, 0x000000, 
        0x000000, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0x000000
    },

    // Número 3
    {
        0x000000, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0x000000, 
        0x000000, 0x000000, 0x000000, 0xFFFFFF, 0x000000, 
        0x000000, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0x000000, 
        0x000000, 0x000000, 0x000000, 0xFFFFFF, 0x000000, 
        0x000000, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0x000000
    },

    // Número 4
    {
        0x000000, 0xFFFFFF, 0x000000, 0x000000, 0x000000, 
        0x000000, 0x000000, 0x000000, 0xFFFFFF, 0x000000, 
        0x000000, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0x000000, 
        0x000000, 0xFFFFFF, 0x000000, 0xFFFFFF, 0x000000, 
        0x000000, 0xFFFFFF, 0x000000, 0xFFFFFF, 0x000000
    },

    // Número 5
    {
        0x000000, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0x000000, 
        0x000000, 0x000000, 0x000000, 0xFFFFFF, 0x000000, 
        0x000000, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0x000000, 
        0x000000, 0xFFFFFF, 0x000000, 0x000000, 0x000000, 
        0x000000, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0x000000
    },

    // Número 6
    {
        0x000000, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0x000000, 
        0x000000, 0xFFFFFF, 0x000000, 0xFFFFFF, 0x000000, 
        0x000000, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0x000000, 
        0x000000, 0xFFFFFF, 0x000000, 0x000000, 0x000000, 
        0x000000, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0x000000
    },

    // Número 7
    {
        0x000000, 0x000000, 0xFFFFFF, 0x000000, 0x000000, 
        0x000000, 0x000000, 0xFFFFFF, 0x000000, 0x000000, 
        0x000000, 0x000000, 0xFFFFFF, 0x000000, 0x000000, 
        0x000000, 0x000000, 0x000000, 0xFFFFFF, 0x000000, 
        0x000000, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0x000000
    },

    // Número 8
    {
        0x000000, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0x000000, 
        0x000000, 0xFFFFFF, 0x000000, 0xFFFFFF, 0x000000, 
        0x000000, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0x000000, 
        0x000000, 0xFFFFFF, 0x000000, 0xFFFFFF, 0x000000, 
        0x000000, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0x000000
    },

    // Número 9
    {
        0x000000, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0x000000, 
        0x000000, 0x000000, 0x000000, 0xFFFFFF, 0x000000, 
        0x000000, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0x000000, 
        0x000000, 0xFFFFFF, 0x000000, 0xFFFFFF, 0x000000, 
        0x000000, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0x000000
    }
};

static void configure_pio() {
    gpio_init(LED_GREEN);
    gpio_init(LED_BLUE);
    gpio_init(LED_RED);
    gpio_init(BUTTON_A);
    gpio_init(BUTTON_B);

    gpio_set_dir(LED_GREEN, GPIO_OUT);
    gpio_set_dir(LED_BLUE, GPIO_OUT);
    gpio_set_dir(LED_RED, GPIO_OUT);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_set_dir(BUTTON_B, GPIO_IN);

    gpio_pull_up(BUTTON_A);
    gpio_pull_up(BUTTON_B);
}

// Função para piscar o LED vermelho 5 vezes por segundo
void blink_red_led() {
    static bool state = false;
    gpio_put(LED_RED, state);
    state = !state;
}

// Função para exibir números na matriz de LEDs
void display_number(int number) {
    for (int i = 0; i < NUM_LEDS; i++) {
        pio_sm_put_blocking(pio, sm, number_patterns[number][i]);
    }
}

// Interrupção para o botão A (incrementa número)
void button_a_irq_handler(uint gpio, uint32_t events) {
    absolute_time_t now = get_absolute_time();
    if (absolute_time_diff_us(last_press_time_A, now) > DEBOUNCE_TIME * 1000) {
        current_number = (current_number + 1) % 10;
        display_number(current_number);
        last_press_time_A = now;
    }
}

// Interrupção para o botão B (decrementa número)
void button_b_irq_handler(uint gpio, uint32_t events) {
    absolute_time_t now = get_absolute_time();
    if (absolute_time_diff_us(last_press_time_B, now) > DEBOUNCE_TIME * 1000) {
        current_number = (current_number - 1 + 10) % 10;
        display_number(current_number);
        last_press_time_B = now;
    }
}

int main() {
    stdio_init_all();
    configure_pio();

    // Ajusta clock para 128 MHz (se desejar)
    bool ok = set_sys_clock_khz(128000, false);
    if (ok) {
        printf("Clock ajustado para %ld Hz\n", clock_get_hz(clk_sys));
    }

    // Inicializa o PIO (ws2812_program)
    uint offset = pio_add_program(pio, &ws2812_program);
    sm = pio_claim_unused_sm(pio, true);
    ws2812_program_init(pio, sm, offset, LED_PIN);

    // Configura as interrupções para os botões
    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, &button_a_irq_handler);
    gpio_set_irq_enabled_with_callback(BUTTON_B, GPIO_IRQ_EDGE_FALL, true, &button_b_irq_handler);

    while (true) {
        blink_red_led();
        sleep_ms(100);
    }
}
