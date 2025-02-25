#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"

// Definição dos pinos do joystick
#define JOYSTICK_X 26 // Pino ADC para eixo X (velocidade)
#define JOYSTICK_Y 27 // Pino ADC para eixo Y (inclinação)
#define JOYSTICK_BUTTON 22 // Pino digital para o botão do joystick

// Definição dos níveis de inclinação e velocidade
const float inclinacao_niveis[] = {0.0, 3.0, 6.0, 9.0, 12.0};
const int velocidade_max = 14; // Máxima velocidade em km/h

// Função para mapear a leitura do ADC para um valor desejado
float mapear_adc(uint16_t valor_adc, float min_saida, float max_saida) {
    return min_saida + (max_saida - min_saida) * (valor_adc / 4095.0);
}

int main() {
    stdio_init_all();
    adc_init();

    // Configura os canais ADC para os eixos do joystick
    adc_gpio_init(JOYSTICK_X);
    adc_gpio_init(JOYSTICK_Y);

    gpio_init(JOYSTICK_BUTTON);
    gpio_set_dir(JOYSTICK_BUTTON, GPIO_IN);
    gpio_pull_up(JOYSTICK_BUTTON);

    while (true) {
        adc_select_input(0);
        uint16_t valor_x = adc_read(); // Leitura do eixo X (velocidade)

        adc_select_input(1);
        uint16_t valor_y = adc_read(); // Leitura do eixo Y (inclinação)

        // Mapeia a velocidade e a inclinação
        float velocidade = mapear_adc(valor_x, 0, velocidade_max);
        int nivel_inclinacao = (int)mapear_adc(valor_y, 0, 4); // Índice do array inclinacao_niveis
        float inclinacao = inclinacao_niveis[nivel_inclinacao];

        // Exibe os valores no terminal (vamos substituir isso pelo display depois)
        printf("Velocidade: %.1f km/h | Inclinação: %.1f%%\n", velocidade, inclinacao);

        sleep_ms(500); // Pequeno delay para evitar leituras rápidas demais
    }

    return 0;
}
