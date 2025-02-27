📄 Documentação do Projeto Final Embarcatech

Este projeto é um sistema embarcado que simula uma esteira de exercícios com controle de velocidade, inclinação e exibição de informações em um display OLED. O sistema utiliza um joystick para controle de velocidade e inclinação, botões para iniciar, pausar e finalizar o treino, e um buzzer para feedback sonoro. O display OLED exibe informações como velocidade, inclinação, distância percorrida e médias durante o treino.

📝 Descrição dos Arquivos

    📄 CMakeLists.txt: Arquivo de configuração do CMake para compilar o projeto.
    📄 LICENSE: Arquivo de licença do projeto.
    📄 README.md: Documentação básica do projeto.
    📄 lib/font.h: Contém a definição de uma fonte para exibição de caracteres no display OLED.
    📄 lib/ssd1306.c: Implementação das funções para controlar o display OLED.
    📄 lib/ssd1306.h: Definição das funções e estruturas para controlar o display OLED.
    📄 pico_sdk_import.cmake: Arquivo de configuração para importar o SDK do Raspberry Pi Pico.
    📄 projeto_final_embarcatech.c: Arquivo principal do projeto, contendo a lógica de controle da esteira e interação com os periféricos.

⚙️ Funcionalidades Principais

    🎮 Controle de Velocidade e Inclinação:
        O joystick é usado para controlar a velocidade e a inclinação da esteira.
        O eixo X do joystick controla a velocidade.
        O eixo Y do joystick controla a inclinação.

    ⏯️ Início, Pausa e Finalização do Treino:
        O botão do joystick é usado para iniciar ou pausar o treino.
        O botão A é usado para finalizar o treino.
        O botão B é usado para ativar/desativar um alerta de emergência.

    🖥️ Exibição de Informações no Display OLED:
        O display OLED exibe informações como velocidade, inclinação, distância percorrida e médias durante o treino.
        Quando o treino é finalizado, o display exibe um resumo do treino e uma mensagem de "TREINO FINALIZADO".

    🔊 Feedback Sonoro:
        O buzzer emite beeps para indicar o início, pausa e finalização do treino.
        Um alerta de emergência é ativado quando o botão B é pressionado, emitindo um som intermitente e acendendo um LED vermelho.

    💡 Controle de Brilho do LED Azul:
        O LED azul é usado para indicar o progresso do treino, com o brilho aumentando conforme o tempo passa.

🛠️ Detalhes de Implementação
📄 projeto_final_embarcatech.c

    🔧 Inicialização:
        Configuração dos pinos do joystick, buzzer, botões e display OLED.
        Inicialização do ADC para leitura dos eixos do joystick.
        Configuração do PWM para controle do brilho do LED azul.

    🔄 Loop Principal:
        Verifica o estado dos botões e do joystick para controlar o treino.
        Atualiza a velocidade e a inclinação com base nas leituras do joystick.
        Atualiza o display OLED com as informações do treino.
        Emite beeps e controla o LED azul conforme o progresso do treino.

    📋 Funções Principais:
        iniciar_treino(): Inicia ou retoma o treino.
        pausar_treino(): Pausa o treino e exibe as médias no display.
        finalizar_treino(): Finaliza o treino, exibe o resumo e a mensagem de finalização.
        atualizar_display_treino(): Atualiza o display com as informações do treino.
        emitir_beeps(): Emite beeps com o buzzer.
        pedir_ajuda_emergencia(): Ativa/desativa o alerta de emergência.

📄 ssd1306.c e ssd1306.h

    🔧 Inicialização do Display:
        Configuração do display OLED via I2C.
        Inicialização do buffer de memória para o display.

    🎨 Funções de Desenho:
        ssd1306_pixel(): Desenha um pixel no display.
        ssd1306_fill(): Preenche o display com um valor (ligado/desligado).
        ssd1306_rect(): Desenha um retângulo no display.
        ssd1306_line(): Desenha uma linha no display.
        ssd1306_draw_char(): Desenha um caractere no display.
        ssd1306_draw_string(): Desenha uma string no display.

📄 font.h

    🔠 Definição da Fonte:
        Contém a definição de uma fonte 8x8 para caracteres alfanuméricos e alguns símbolos especiais.

🛠️ Compilação e Execução

Para compilar o projeto, utilize o CMake com o SDK do Raspberry Pi Pico:
bash
Copy

mkdir build
cd build
cmake ..
make

O arquivo binário gerado pode ser carregado no Raspberry Pi Pico para execução.
🏁 Considerações Finais

Este projeto demonstra a integração de vários periféricos em um sistema embarcado, incluindo controle de entrada/saída, comunicação I2C, e exibição gráfica. A estrutura modular do código facilita a expansão e manutenção do sistema.

📹 Vídeo do Projeto

Para visualizar uma demonstração do projeto em funcionamento, confira o vídeo:
[https://youtu.be/lhOVhGRdH_g](https://youtu.be/lhOVhGRdH_g)