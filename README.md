# 📌 Projeto: Controle de Matriz de LEDs WS2812 com Interrupções no RP2040

## 📖 Sobre o Projeto
Este projeto tem como objetivo **explorar o uso de interrupções no microcontrolador RP2040** da **placa BitDogLab**, utilizando **botões e LEDs WS2812** para criar uma interface interativa. Os principais conceitos abordados incluem **tratamento de interrupções, debounce via software e controle de LEDs endereçáveis.**

## 🎯 Objetivos do Projeto
✅ Compreender o funcionamento de **interrupções** em microcontroladores.  
✅ Implementar **debouncing via software** para evitar efeitos indesejados ao pressionar botões.  
✅ Manipular e controlar **LEDs comuns e LEDs endereçáveis WS2812**.  
✅ Utilizar **resistores de pull-up internos** nos botões de acionamento.  
✅ Desenvolver um sistema funcional que **integra hardware e software**.  

## 🛠️ Componentes Utilizados
- **Microcontrolador:** RP2040 (Placa BitDogLab)  
- **Matriz de LEDs 5x5 WS2812:** Conectada à **GPIO 7**  
- **LED RGB comum:** Conectado às **GPIOs 11, 12 e 13**  
- **Botão A:** Conectado à **GPIO 5**  
- **Botão B:** Conectado à **GPIO 6**  

## 🚀 Funcionalidades Implementadas
✅ **LED Vermelho piscando continuamente a 5Hz** (5 vezes por segundo).  
✅ **Botão A incrementa** o número exibido na matriz de LEDs.  
✅ **Botão B decrementa** o número exibido na matriz de LEDs.  
✅ **Os LEDs WS2812 exibem números de 0 a 9** em um formato digital fixo ou estilizado.  
✅ **Uso de interrupções (IRQ) para capturar eventos de botões** e evitar polling contínuo.  
✅ **Tratamento de debounce via software** para evitar múltiplos acionamentos inesperados.  

## 🛠️ Configuração do Hardware
1. **Conectar os componentes à BitDogLab** conforme a seguinte tabela:

| Componente | GPIO |
|------------|------|
| Matriz de LEDs WS2812 | 7 |
| LED RGB - Verde | 11 |
| LED RGB - Azul | 12 |
| LED RGB - Vermelho | 13 |
| Botão A | 5 |
| Botão B | 6 |

2. **Ativar os resistores de pull-up internos** para os botões no código.
3. **Configurar o PIO para controlar a matriz WS2812** com eficiência.

## 🖥️ Uso do Projeto
- O **LED vermelho pisca automaticamente**.
- Pressione **Botão A** ➡ Incrementa o número na matriz.
- Pressione **Botão B** ➡ Decrementa o número na matriz.
- A matriz exibe os números em **formato digital**.

## 🔍 Trechos de Código Destacados
### **Configuração da Interrupção para os Botões**
```c
void button_irq_handler(uint gpio, uint32_t events) {
    absolute_time_t now = get_absolute_time();
    if (gpio == BUTTON_A) {
        if (absolute_time_diff_us(last_press_time_A, now) > DEBOUNCE_TIME * 1000) {
            current_number = (current_number + 1) % 10;
            display_number(current_number);
            last_press_time_A = now;
            printf("Botão A pressionado. Número atual: %d\n", current_number);
        }
    } else if (gpio == BUTTON_B) {
        if (absolute_time_diff_us(last_press_time_B, now) > DEBOUNCE_TIME * 1000) {
            current_number = (current_number - 1 + 10) % 10;
            display_number(current_number);
            last_press_time_B = now;
            printf("Botão B pressionado. Número atual: %d\n", current_number);
        }
    }
    gpio_acknowledge_irq(gpio, events);
}
```

### **Piscar o LED Vermelho a 5Hz**
```c
void blink_red_led() {
    static bool state = false;
    gpio_put(LED_RED, state);
    state = !state;
}
```
### **Vídeo de demonstração do funcionamento do código**
- **[Link para acessar o vídeo]

## 🔗 Referências
- **[Documentação do RP2040](https://datasheets.raspberrypi.com/rp2040/rp2040-datasheet.pdf)**
- **[Guia sobre interrupções no RP2040](https://github.com/raspberrypi/pico-sdk)**
- **[BitDogLab Docs](https://bitdoglab.com/)**

## 📜 Licença
Este projeto é de código aberto e pode ser usado livremente para fins educacionais e experimentação.

---

🚀 **Agora é só compilar e testar o projeto!** Caso tenha dúvidas, entre em contato. 😃
