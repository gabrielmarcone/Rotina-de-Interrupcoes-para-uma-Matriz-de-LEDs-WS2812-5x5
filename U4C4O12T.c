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

// Variável global para cor atual
uint32_t color;

// Tabela de padrões para exibição de números na matriz WS2812
const double number_patterns[10][NUM_LEDS] = {
    // Número 0
    {
        0.0, 0.0, 0.3, 0.0, 0.0, 
        0.0, 0.3, 0.0, 0.3, 0.0, 
        0.0, 0.3, 0.0, 0.3, 0.0, // 30% de intensidade  
        0.0, 0.3, 0.0, 0.3, 0.0, 
        0.0, 0.0, 0.3, 0.0, 0.0
     },
    
    // Número 0.3
    {
        0.0, 0.0, 0.3, 0.0, 0.0, 
        0.0, 0.0, 0.3, 0.0, 0.0, 
        0.0, 0.0, 0.3, 0.0, 0.0, 
        0.0, 0.3, 0.3, 0.0, 0.0, 
        0.0, 0.0, 0.3, 0.0, 0.0
    },

    // Número 2
    {
        0.0, 0.3, 0.3, 0.3, 0.0, 
        0.0, 0.3, 0.0, 0.0, 0.0, 
        0.0, 0.3, 0.3, 0.3, 0.0, 
        0.0, 0.0, 0.0, 0.3, 0.0, 
        0.0, 0.3, 0.3, 0.3, 0.0
    },

    // Número 3
    {
        0.0, 0.3, 0.3, 0.3, 0.0, 
        0.0, 0.0, 0.0, 0.3, 0.0, 
        0.0, 0.3, 0.3, 0.3, 0.0, 
        0.0, 0.0, 0.0, 0.3, 0.0, 
        0.0, 0.3, 0.3, 0.3, 0.0
    },

    // Número 4
    {
        0.0, 0.3, 0.0, 0.0, 0.0, 
        0.0, 0.0, 0.0, 0.3, 0.0, 
        0.0, 0.3, 0.3, 0.3, 0.0, 
        0.0, 0.3, 0.0, 0.3, 0.0, 
        0.0, 0.3, 0.0, 0.3, 0.0
    },

    // Número 5
    {
        0.0, 0.3, 0.3, 0.3, 0.0, 
        0.0, 0.0, 0.0, 0.3, 0.0, 
        0.0, 0.3, 0.3, 0.3, 0.0, 
        0.0, 0.3, 0.0, 0.0, 0.0, 
        0.0, 0.3, 0.3, 0.3, 0.0
    },

    // Número 6
    {
        0.0, 0.3, 0.3, 0.3, 0.0, 
        0.0, 0.3, 0.0, 0.3, 0.0, 
        0.0, 0.3, 0.3, 0.3, 0.0, 
        0.0, 0.3, 0.0, 0.0, 0.0, 
        0.0, 0.3, 0.3, 0.3, 0.0
    },

    // Número 7
    {
        0.0, 0.0, 0.3, 0.0, 0.0, 
        0.0, 0.0, 0.3, 0.0, 0.0, 
        0.0, 0.0, 0.3, 0.0, 0.0, 
        0.0, 0.0, 0.0, 0.3, 0.0, 
        0.0, 0.3, 0.3, 0.3, 0.0
    },

    // Número 8
    {
        0.0, 0.3, 0.3, 0.3, 0.0, 
        0.0, 0.3, 0.0, 0.3, 0.0, 
        0.0, 0.3, 0.3, 0.3, 0.0, 
        0.0, 0.3, 0.0, 0.3, 0.0, 
        0.0, 0.3, 0.3, 0.3, 0.0
    },

    // Número 9
    {
        0.0, 0.3, 0.3, 0.3, 0.0, 
        0.0, 0.0, 0.0, 0.3, 0.0, 
        0.0, 0.3, 0.3, 0.3, 0.0, 
        0.0, 0.3, 0.0, 0.3, 0.0, 
        0.0, 0.3, 0.3, 0.3, 0.0
    }
};

// Converte intensidade (0.0..1.0) de cada canal G, R, B em 24 bits no formato:
// ( G << 24 ) | ( R << 16 ) | ( B << 8 )
uint32_t matrix_rgb(double b, double r, double g) {
    unsigned char R = (unsigned char)(r * 255.0);
    unsigned char G = (unsigned char)(g * 255.0);
    unsigned char B = (unsigned char)(b * 255.0);

    return ( (uint32_t)G << 24 ) |
           ( (uint32_t)R << 16 ) |
           ( (uint32_t)B <<  8 );
}

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
        double intensity = number_patterns[number][i]; // Intensidade do LED
        color = matrix_rgb(intensity, 0, intensity); // Ciano
        pio_sm_put_blocking(pio, sm, color);
    }
}

// **Callback ÚNICO para interrupções de botões**
void button_irq_handler(uint gpio, uint32_t events) {
    absolute_time_t now = to_us_since_boot(get_absolute_time());

    if (gpio == BUTTON_A) {  // **Botão A: Incrementa**
        if (now - last_press_time_A > DEBOUNCE_TIME * 1000) {
            current_number = (current_number + 1) % 10;
            display_number(current_number);
            last_press_time_A = now;
            printf("Botão A pressionado. Número atual: %d\n", current_number);
        }
    } 
    else if (gpio == BUTTON_B) {  // **Botão B: Decrementa**
        if (now - last_press_time_B > DEBOUNCE_TIME * 1000) {
            current_number = (current_number - 1 + 10) % 10;
            display_number(current_number);
            last_press_time_B = now;
            printf("Botão B pressionado. Número atual: %d\n", current_number);
        }
    }

    gpio_acknowledge_irq(gpio, events);  // Confirma que a interrupção foi tratada
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
    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, &button_irq_handler);
    gpio_set_irq_enabled_with_callback(BUTTON_B, GPIO_IRQ_EDGE_FALL, true, &button_irq_handler);

    while (true) {
        blink_red_led();
        sleep_ms(100);
    }
}