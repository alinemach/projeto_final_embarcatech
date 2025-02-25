#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"

// Definição dos pinos do joystick e buzzer
#define JOYSTICK_X 26      // Pino ADC para eixo X (velocidade)
#define JOYSTICK_Y 27      // Pino ADC para eixo Y (inclinação)
#define JOYSTICK_BUTTON 22 // Pino digital para o botão do joystick
#define BUZZER 21          // Pino digital para o buzzer

//Trecho para modo BOOTSEL com botão B
#include "pico/bootrom.h"
#define Botao_B 6
void gpio_irq_handler(uint gpio, uint32_t events)
{
  reset_usb_boot(0, 0);
}

// Definição dos níveis de inclinação e velocidade
const float inclinacao_niveis[] = {0.0, 3.0, 6.0, 9.0, 12.0};
const int velocidade_max = 14; // Máxima velocidade em km/h

// Variáveis globais para controle do treino
float velocidade_atual = 0.0;
float inclinacao_atual = 0.0;
float distancia_percorrida = 0.0;
float soma_velocidade = 0.0;
float soma_inclinacao = 0.0;
int contador_medidas = 0;
bool treino_em_andamento = false;
bool treino_pausado = false;
absolute_time_t tempo_anterior;
int indice_inclinacao = 0; // Índice da inclinação inicial

// Função para emitir beeps com o buzzer
void emitir_beeps(int quantidade, int duracao, int intervalo) {
    for (int i = 0; i < quantidade; i++) {
        gpio_put(BUZZER, 1);
        sleep_ms(duracao);
        gpio_put(BUZZER, 0);
        sleep_ms(intervalo);
    }
}

// Função para iniciar ou retomar o treino
void iniciar_treino() {
    printf("Iniciando treino...\n");
    emitir_beeps(2, 1000, 500); // 2 beeps de 1s com 0.5s de intervalo
    emitir_beeps(1, 3000, 0);   // 1 beep longo de 3s
    treino_em_andamento = true;
    treino_pausado = false;
    distancia_percorrida = 0.0;
    soma_velocidade = 0.0;
    soma_inclinacao = 0.0;
    contador_medidas = 0;
    
    // Resetando velocidade e inclinação
    velocidade_atual = 0.0;
    indice_inclinacao = 0;
    inclinacao_atual = inclinacao_niveis[indice_inclinacao];

    tempo_anterior = get_absolute_time();
}

// Função para pausar o treino
void pausar_treino() {
    printf("Treino pausado!\n");
    emitir_beeps(1, 2000, 0); // 1 beep de 2s
    printf("Distância percorrida: %.1f m\n", distancia_percorrida);
    treino_pausado = true;
}

// Função para finalizar o treino
void finalizar_treino() {
    printf("Treino finalizado!\n");
    emitir_beeps(4, 500, 300); // 4 beeps curtos
    treino_em_andamento = false;
    treino_pausado = false;

    // Cálculo das médias
    float velocidade_media = contador_medidas > 0 ? soma_velocidade / contador_medidas : 0.0;
    float inclinacao_media = contador_medidas > 0 ? soma_inclinacao / contador_medidas : 0.0;

    printf("Resumo do treino:\n");
    printf("Distância percorrida: %.1f m\n", distancia_percorrida);
    printf("Velocidade média: %.1f km/h\n", velocidade_media);
    printf("Inclinação média: %.1f%%\n", inclinacao_media);
}

// Função para mapear leitura do ADC para um valor desejado
float mapear_adc(uint16_t valor_adc, float min_saida, float max_saida) {
    return min_saida + (max_saida - min_saida) * (valor_adc / 4095.0);
}

int main() {
    stdio_init_all();
    adc_init();

    // Configuração do modo BOOTSEL com botão B
    gpio_init(Botao_B);
    gpio_set_dir(Botao_B, GPIO_IN);
    gpio_pull_up(Botao_B);
    gpio_set_irq_enabled_with_callback(Botao_B, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    // Configura os pinos do joystick e do buzzer
    adc_gpio_init(JOYSTICK_X);
    adc_gpio_init(JOYSTICK_Y);

    gpio_init(JOYSTICK_BUTTON);
    gpio_set_dir(JOYSTICK_BUTTON, GPIO_IN);
    gpio_pull_up(JOYSTICK_BUTTON);

    gpio_init(BUZZER);
    gpio_set_dir(BUZZER, GPIO_OUT);
    gpio_put(BUZZER, 0);

    velocidade_atual = 0.0;
    tempo_anterior = get_absolute_time();

    while (true) {
        if (!treino_em_andamento) {
            // Espera o botão ser pressionado por 1 segundo para iniciar o treino
            if (!gpio_get(JOYSTICK_BUTTON)) {
                sleep_ms(1000);
                if (!gpio_get(JOYSTICK_BUTTON)) {
                    iniciar_treino();
                }
            }
        } else {
            if (!gpio_get(JOYSTICK_BUTTON)) {
                sleep_ms(200);
                if (!gpio_get(JOYSTICK_BUTTON)) {
                    uint32_t tempo_inicio = to_ms_since_boot(get_absolute_time());

                    // Espera para verificar se há um segundo clique dentro de 500ms
                    while (to_ms_since_boot(get_absolute_time()) - tempo_inicio < 500) {
                        if (!gpio_get(JOYSTICK_BUTTON)) {
                            finalizar_treino();
                            break;
                        }
                    }

                    if (!treino_em_andamento) {
                        continue;
                    }

                    // Se não houve clique duplo, pausa ou retoma o treino
                    if (treino_pausado) {
                        iniciar_treino();
                    } else {
                        pausar_treino();
                    }
                }
            }
        }

        if (treino_em_andamento && !treino_pausado) {
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

            // Atualiza as somas para cálculo da média
            soma_velocidade += velocidade_atual;
            soma_inclinacao += inclinacao_atual;
            contador_medidas++;

            // Calcula a distância percorrida
            absolute_time_t tempo_atual = get_absolute_time();
            int64_t tempo_decorrido_ms = absolute_time_diff_us(tempo_anterior, tempo_atual) / 1000;
            tempo_anterior = tempo_atual;

            distancia_percorrida += (velocidade_atual * 1000 / 3600) * (tempo_decorrido_ms / 1000.0);

            printf("Velocidade: %.1f km/h | Inclinação: %.1f%% | Distância: %.1f m\n",
                   velocidade_atual, inclinacao_atual, distancia_percorrida);

            sleep_ms(500);
        }
    }

    return 0;
}
