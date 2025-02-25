#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/timer.h"

// Definição dos pinos do joystick
#define JOYSTICK_X 26      // Pino ADC para eixo X (velocidade)
#define JOYSTICK_Y 27      // Pino ADC para eixo Y (inclinação)
#define JOYSTICK_BUTTON 22 // Pino digital para o botão do joystick

// Trecho para modo BOOTSEL com botão B
#include "pico/bootrom.h"
#define Botao_B 6

// Variável global para debouncing
static volatile uint32_t last_button_time = 0; // Armazena o tempo do último evento (em microssegundos)

// Função de interrupção para o botão B (BOOTSEL)
void gpio_irq_handler_bootsel(uint gpio, uint32_t events) {
    reset_usb_boot(0, 0);
}

// Função de interrupção para o botão do joystick com debouncing
void gpio_irq_handler_joystick(uint gpio, uint32_t events) {
    // Obtém o tempo atual em microssegundos
    uint32_t current_time = to_us_since_boot(get_absolute_time());

    // Verifica se passou tempo suficiente desde o último evento
    if (current_time - last_button_time > 200000) { // 200 ms de debouncing
        last_button_time = current_time; // Atualiza o tempo do último evento

        // Aqui você pode adicionar a lógica para tratar o pressionamento do botão do joystick
        printf("Botão do joystick pressionado!\n");
    }
}

// Definição dos níveis de inclinação e velocidade
const float inclinacao_niveis[] = {0.0, 3.0, 6.0, 9.0, 12.0};
const int velocidade_max = 14; // Máxima velocidade em km/h

// Variáveis para manter valores constantes
float velocidade_atual = 0.0;
float inclinacao_atual = 0.0;
float distancia_percorrida = 0.0;
absolute_time_t tempo_anterior;

// Função para mapear a leitura do ADC para um valor desejado
float mapear_adc(uint16_t valor_adc, float min_saida, float max_saida) {
    return min_saida + (max_saida - min_saida) * (valor_adc / 4095.0);
}

int main() {
    stdio_init_all();
    adc_init();

    // Para ser utilizado o modo BOOTSEL com botão B
    gpio_init(Botao_B);
    gpio_set_dir(Botao_B, GPIO_IN);
    gpio_pull_up(Botao_B);
    gpio_set_irq_enabled_with_callback(Botao_B, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler_bootsel);

    // Configura os canais ADC para os eixos do joystick
    adc_gpio_init(JOYSTICK_X);
    adc_gpio_init(JOYSTICK_Y);

    // Configura o botão do joystick com interrupção e debouncing
    gpio_init(JOYSTICK_BUTTON);
    gpio_set_dir(JOYSTICK_BUTTON, GPIO_IN);
    gpio_pull_up(JOYSTICK_BUTTON);
    gpio_set_irq_enabled_with_callback(JOYSTICK_BUTTON, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler_joystick);

    int indice_inclinacao = 0; // Começa em 0%
    velocidade_atual = 0.0;    // Começa em 0 km/h
    tempo_anterior = get_absolute_time();

    while (true) {
        adc_select_input(0);
        uint16_t valor_x = adc_read(); // Leitura do eixo X (velocidade)

        adc_select_input(1);
        uint16_t valor_y = adc_read(); // Leitura do eixo Y (inclinação)

        // Atualiza a inclinação apenas se o joystick for movido para cima ou para baixo
        if (valor_y > 3000 && indice_inclinacao < 4) {
            indice_inclinacao++;
        } else if (valor_y < 1000 && indice_inclinacao > 0) {
            indice_inclinacao--;
        }
        inclinacao_atual = inclinacao_niveis[indice_inclinacao];

        // Atualiza a velocidade apenas se o joystick for movido para os lados
        if (valor_x > 3000 && velocidade_atual < velocidade_max) {
            velocidade_atual += 0.5;
        } else if (valor_x < 1000 && velocidade_atual > 0.0) {
            velocidade_atual -= 0.5;
        }

        // Calcula a distância percorrida
        absolute_time_t tempo_atual = get_absolute_time();
        int64_t tempo_decorrido_ms = absolute_time_diff_us(tempo_anterior, tempo_atual) / 1000;
        tempo_anterior = tempo_atual;

        // Converte km/h para m/s e atualiza a distância
        distancia_percorrida += (velocidade_atual * 1000 / 3600) * (tempo_decorrido_ms / 1000.0);

        // Exibe os valores no terminal
        printf("Velocidade: %.1f km/h | Inclinação: %.1f%% | Distância: %.1f m\n", velocidade_atual, inclinacao_atual, distancia_percorrida);

        sleep_ms(500); // Pequeno delay para evitar leituras rápidas demais
    }

    return 0;
}