#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"

// Definição dos pinos do joystick, buzzer e botão A
#define JOYSTICK_X 26      // Pino ADC para eixo X (velocidade)
#define JOYSTICK_Y 27      // Pino ADC para eixo Y (inclinação)
#define JOYSTICK_BUTTON 22 // Pino digital para o botão do joystick
#define BUZZER 21          // Pino digital para o buzzer
#define BOTAO_A 5         // Pino digital para o botão A

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
uint32_t tempo_ultimo_clique_botao_A = 0; // Tempo do último clique no botão A
bool debounce_botao_A = false; // Flag para debouncing do botão A

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
    if (!treino_pausado) {  // Apenas zera os valores se for um treino novo
        printf("Iniciando novo treino...\n");
        distancia_percorrida = 0.0;
        soma_velocidade = 0.0;
        soma_inclinacao = 0.0;
        contador_medidas = 0;
        velocidade_atual = 0.0;
        indice_inclinacao = 0;
        inclinacao_atual = inclinacao_niveis[indice_inclinacao];
    } else {
        printf("Retomando treino...\n");
    }
    
    emitir_beeps(2, 1000, 500);
    treino_em_andamento = true;
    treino_pausado = false;
    tempo_anterior = get_absolute_time();  // Retoma a contagem de tempo corretamente
}

void calcula_medias(){
    // Cálculo das médias
    float velocidade_media = contador_medidas > 0 ? soma_velocidade / contador_medidas : 0.0;
    float inclinacao_media = contador_medidas > 0 ? soma_inclinacao / contador_medidas : 0.0;

    // Imprimir as médias
    printf("Velocidade média: %.1f km/h\n", velocidade_media);
    printf("Inclinação média: %.1f%%\n", inclinacao_media);
}

// Função para pausar o treino
void pausar_treino() {
    printf("Treino pausado!\n");
    emitir_beeps(1, 2000, 0); // 1 beep de 2s
    printf("Distância percorrida: %.1f m\n", distancia_percorrida);
    calcula_medias();  // Chama a função para calcular e imprimir as médias
    treino_pausado = true;
}

// Função para finalizar o treino
void finalizar_treino() {
    printf("Treino finalizado!\n");
    emitir_beeps(4, 500, 300); // 4 beeps curtos
    treino_em_andamento = false;
    treino_pausado = false;

    printf("Resumo do treino:\n");
    printf("Distância percorrida: %.1f m\n", distancia_percorrida);
    calcula_medias();  // Chama a função para calcular e imprimir as médias
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
    
    // Configura os pinos do joystick, buzzer e botão A
    adc_gpio_init(JOYSTICK_X);
    adc_gpio_init(JOYSTICK_Y);

    gpio_init(JOYSTICK_BUTTON);
    gpio_set_dir(JOYSTICK_BUTTON, GPIO_IN);
    gpio_pull_up(JOYSTICK_BUTTON);

    gpio_init(BUZZER);
    gpio_set_dir(BUZZER, GPIO_OUT);
    gpio_put(BUZZER, 0);

    gpio_init(BOTAO_A);
    gpio_set_dir(BOTAO_A, GPIO_IN);
    gpio_pull_up(BOTAO_A);

    velocidade_atual = 0.0;
    tempo_anterior = get_absolute_time();

    while (true) {
        if (!treino_em_andamento) {
            // Espera o botão do joystick ser pressionado por 1 segundo para iniciar o treino
            if (!gpio_get(JOYSTICK_BUTTON)) {
                uint32_t tempo_inicio = to_ms_since_boot(get_absolute_time());
                while (!gpio_get(JOYSTICK_BUTTON) && to_ms_since_boot(get_absolute_time()) - tempo_inicio < 1000) {
                    // Aguarda 1 segundo pressionado
                }
                if (!gpio_get(JOYSTICK_BUTTON)) {
                    iniciar_treino();
                }
            }
        } else {
            // Verifica o botão A para finalizar o treino
            if (!gpio_get(BOTAO_A)) {
                uint32_t tempo_atual = to_ms_since_boot(get_absolute_time());
                if (tempo_atual - tempo_ultimo_clique_botao_A > 200) { // Debouncing de 200ms
                    finalizar_treino();
                    tempo_ultimo_clique_botao_A = tempo_atual;
                }
            }

            // Verifica o botão do joystick para pausar ou retomar o treino
            if (!gpio_get(JOYSTICK_BUTTON)) {
                uint32_t tempo_atual = to_ms_since_boot(get_absolute_time());
                if (tempo_atual - tempo_ultimo_clique_botao_A > 200) { // Debouncing de 200ms
                    if (treino_pausado) {
                        iniciar_treino();
                    } else {
                        pausar_treino();
                    }
                    tempo_ultimo_clique_botao_A = tempo_atual;
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