# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.1)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
# Configuração mínima do CMake
cmake_minimum_required(VERSION 3.13)

# Definição dos padrões de linguagem
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Definição da placa (pico_w para Raspberry Pi Pico W)
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Inclusão do SDK do Raspberry Pi Pico
include(pico_sdk_import.cmake)

# Definição do projeto
project(projeto_final_embarcatech C CXX ASM)

# Inicialização do SDK
pico_sdk_init()

# Adição dos arquivos fonte ao executável
add_executable(projeto_final_embarcatech
    projeto_final_embarcatech.c
    lib/ssd1306.c
)

# Configuração do nome e versão do programa
pico_set_program_name(projeto_final_embarcatech "projeto_final_embarcatech")
pico_set_program_version(projeto_final_embarcatech "0.1")

# Habilita saída padrão (stdio) via UART e USB
pico_enable_stdio_uart(projeto_final_embarcatech 1)
pico_enable_stdio_usb(projeto_final_embarcatech 1)

# Adiciona a biblioteca padrão do Pico ao projeto
target_link_libraries(projeto_final_embarcatech
    pico_stdlib
    hardware_i2c
    hardware_adc
    hardware_pwm
)

# Adiciona diretórios de inclusão (headers)
target_include_directories(projeto_final_embarcatech PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
  

)

# Gera arquivos adicionais (UF2, HEX, etc.)
pico_add_extra_outputs(projeto_final_embarcatech)