#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"

#include "pio_matrix.pio.h"
#include "lib/ssd1306.h"
#include "lib/font.h"
#include "lib/leds.h"
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C
#define ADC_PIN 28 // GPIO para o voltímetro
#define Botao_A 5  // GPIO para botão A

int R_conhecido = 10000;   // Resistor de 10k ohm
float R_x = 0.0;           // Resistor desconhecido
float ADC_VREF = 3.28;     // Tensão de referência do ADC
int ADC_RESOLUTION = 4095; // Resolução do ADC (12 bits)

// Trecho para modo BOOTSEL com botão B
#include "pico/bootrom.h"
#define botaoB 6

// inicializacao da PIO
void PIO_setup(PIO *pio, uint *sm);
// aproxima o valor lido do ADC para o mais próximo da série E24
float aproxima_E24_com_tolerancia(float valor);
// traduz o valor lido do ADC para a cor correspondente
void obter_codigo_de_cores(int resistencia, char saida[3][20], color_options *cores);

void gpio_irq_handler(uint gpio, uint32_t events)
{
    reset_usb_boot(0, 0);
}

int main()
{
    // variaveis relacionadas a matriz de led
    PIO pio;
    uint sm;
    stdio_init_all();
    // Para ser utilizado o modo BOOTSEL com botão B
    gpio_init(botaoB);
    gpio_set_dir(botaoB, GPIO_IN);
    gpio_pull_up(botaoB);
    gpio_set_irq_enabled_with_callback(botaoB, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    // Aqui termina o trecho para modo BOOTSEL com botão B

    gpio_init(Botao_A);
    gpio_set_dir(Botao_A, GPIO_IN);
    gpio_pull_up(Botao_A);

    // inicializa a PIO
    PIO_setup(&pio, &sm);
    // função de teste
    test_matrix(pio, sm); // faz uma animação indicando que está tudo OK e logo apaga a matriz

    // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, 400 * 1000);

    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);                    // Set the GPIO pin function to I2C
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);                    // Set the GPIO pin function to I2C
    gpio_pull_up(I2C_SDA);                                        // Pull up the data line
    gpio_pull_up(I2C_SCL);                                        // Pull up the clock line
    ssd1306_t ssd;                                                // Inicializa a estrutura do display
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT); // Inicializa o display
    ssd1306_config(&ssd);                                         // Configura o display
    ssd1306_send_data(&ssd);                                      // Envia os dados para o display

    // Limpa o display. O display inicia com todos os pixels apagados.
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);

    adc_init();
    adc_gpio_init(ADC_PIN); // GPIO 28 como entrada analógica

    float tensao;
    char str_x[5]; // Buffer para armazenar a string
    char str_y[5]; // Buffer para armazenar a string

    bool cor = true;
    while (true)
    {
        adc_select_input(2); // Seleciona o ADC para eixo X. O pino 28 como entrada analógica

        float soma = 0.0f;
        for (int i = 0; i < 500; i++)
        {
            soma += adc_read();
            sleep_ms(1);
        }
        float media = soma / 500.0f;

        // Fórmula simplificada: R_x = R_conhecido * ADC_encontrado /(ADC_RESOLUTION - adc_encontrado)
        R_x = (R_conhecido * media) / (ADC_RESOLUTION - media);
        float R_x_aproximado = aproxima_E24_com_tolerancia(R_x);

        char codigo_cores[3][20];
        color_options cores[3];
        obter_codigo_de_cores((int)R_x_aproximado, codigo_cores, cores);
        // desenha a linha com as cores correspondentes na matriz de leds
        draw_line(pio, sm, cores);
        printf("resultado: %f", R_x);

        sprintf(str_x, "%1.0f", media);          // Converte o inteiro em string
        sprintf(str_y, "%1.0f", R_x_aproximado); // Converte o float em string

        //  Atualiza o conteúdo do display com animações
        ssd1306_fill(&ssd, !cor);                          // Limpa o display
        ssd1306_rect(&ssd, 3, 3, 122, 60, cor, !cor);      // Desenha um retângulo
        ssd1306_line(&ssd, 3, 25, 123, 25, cor);           // Desenha uma linha
        ssd1306_line(&ssd, 3, 37, 123, 37, cor);           // Desenha uma linha
        ssd1306_draw_string(&ssd, "CEPEDI   TIC37", 8, 6); // Desenha uma string
        ssd1306_draw_string(&ssd, "EMBARCATECH", 20, 16);  // Desenha uma string
        ssd1306_draw_string(&ssd, "  Ohmimetro", 10, 28);  // Desenha uma string
        ssd1306_draw_string(&ssd, "ADC", 13, 41);          // Desenha uma string
        ssd1306_draw_string(&ssd, "Resisten.", 50, 41);    // Desenha uma string
        ssd1306_line(&ssd, 44, 37, 44, 60, cor);           // Desenha uma linha vertical
        ssd1306_draw_string(&ssd, str_x, 8, 52);           // Desenha uma string
        ssd1306_draw_string(&ssd, str_y, 59, 52);          // Desenha uma string
        ssd1306_send_data(&ssd);                           // Atualiza o display

        sleep_ms(500);
        ssd1306_fill(&ssd, !cor); // Limpa o display
        // exibe os valores de resistência
        ssd1306_draw_string(&ssd, codigo_cores[0], 8, 8);
        ssd1306_draw_string(&ssd, codigo_cores[1], 8, 16);
        ssd1306_draw_string(&ssd, codigo_cores[2], 8, 24);
        ssd1306_send_data(&ssd); // Atualiza o display

        sleep_ms(700);
    }
}

void PIO_setup(PIO *pio, uint *sm)
{
    // configurações da PIO
    *pio = pio0;
    uint offset = pio_add_program(*pio, &pio_matrix_program);
    *sm = pio_claim_unused_sm(*pio, true);
    pio_matrix_program_init(*pio, *sm, offset, LED_PIN);
}

float aproxima_E24_com_tolerancia(float valor)
{
    // Definição da série E24
    const float E24_series[] = {
        1.0, 1.1, 1.2, 1.3, 1.5, 1.6, 1.8, 2.0,
        2.2, 2.4, 2.7, 3.0, 3.3, 3.6, 3.9, 4.3,
        4.7, 5.1, 5.6, 6.2, 6.8, 7.5, 8.2, 9.1};

    float melhor_aproximacao = valor;
    float menor_diferenca = 999999;
    bool encontrou_na_tolerancia = false;

    float fator = 1.0;
    while (valor / fator >= 10.0)
    {
        fator *= 10.0;
    }
    while (valor / fator < 1.0)
    {
        fator /= 10.0;
    }

    // Testa E24 da década atual
    for (int i = 0; i < 24; i++)
    {
        float valor_E24 = E24_series[i] * fator;
        // Limites de tolerância de 5%
        float limite_inferior = valor_E24 * 0.95;
        float limite_superior = valor_E24 * 1.05;

        if (valor >= limite_inferior && valor <= limite_superior)
        {
            float diferenca = fabs(valor - valor_E24);
            if (diferenca < menor_diferenca)
            {
                menor_diferenca = diferenca;
                melhor_aproximacao = valor_E24;
                encontrou_na_tolerancia = true;
            }
        }
    }

    // Testa E24 da próxima década (fator * 10)
    fator *= 10.0;
    for (int i = 0; i < 24; i++)
    {
        float valor_E24 = E24_series[i] * fator;
        float limite_inferior = valor_E24 * 0.95;
        float limite_superior = valor_E24 * 1.05;

        if (valor >= limite_inferior && valor <= limite_superior)
        {
            float diferenca = fabs(valor - valor_E24);
            if (diferenca < menor_diferenca)
            {
                menor_diferenca = diferenca;
                melhor_aproximacao = valor_E24;
                encontrou_na_tolerancia = true;
            }
        }
    }

    return melhor_aproximacao;
}

void obter_codigo_de_cores(int resistencia, char saida[3][20], color_options *cores)
{
    if (resistencia < 10 || resistencia > 990000)
    {
        strcpy(saida[0], "Fora alcance");
        strcpy(saida[1], "");
        strcpy(saida[2], "");
        return;
    }

    int primeira, segunda, multiplicador;
    primeira = segunda = multiplicador = 0;
    int pot = 0;

    while (resistencia >= 100)
    {
        resistencia /= 10;
        pot++;
    }

    primeira = resistencia / 10;
    segunda = resistencia % 10;
    multiplicador = pot;

    cores[0] = primeira;
    cores[1] = segunda;
    cores[2] = multiplicador;

    printf("cores: %d %d %d\n", primeira, segunda, multiplicador);

    // sprintf(saida, "%s %s %s",
    //         color_names[primeira],
    //         color_names[segunda],
    //         color_names[multiplicador]);

    sprintf(saida[0], "%s", color_names[primeira]);
    sprintf(saida[1], "%s", color_names[segunda]);
    sprintf(saida[2], "%s", color_names[multiplicador]);
}