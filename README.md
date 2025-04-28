# ohmímetro com Display OLED, MATRIZ de LEDS e Pico RP2040

Projeto de um ohmímetro digital utilizando o microcontrolador Raspberry Pi Pico, presente na placa BitdogLab, que realiza a leitura de resistores, aproxima o valor para a série comercial E24 (tolerância de 5%), e exibe o valor numérico e o código de cores no display OLED SSD1306 e faz uma animação na matriz de leds exibindo as cores correspondentes.

---

# Funcionalidades

- Leitura da resistência de um resistor conectado ao circuito utilizando o ADC na GPIO 28.

- Cálculo da resistência usando um divisor de tensão conhecido.

- Aproximação automática do valor medido para o valor comercial mais próximo da série E24 (tolerância de 5%).

- Conversão do valor aproximado para o código de cores padrão de resistores.

- Exibição no display OLED:

  - Valor numérico da resistência aproximada.

  - Código de cores (três primeiras faixas).

  - Animações e gráficos auxiliares.

- Animação utilizando a matriz de LEDs

---

# Componentes necessários

- Raspberry Pi Pico.
- Display OLED 128x64 pixels (SSD1306, interface I2C).
- Matriz de LEDs.
- Resistores padrão E24 para testes (510Ω a 100kΩ).

---

# Como funciona o cálculo da resistência

- Um resistor conhecido (10kΩ) é conectado em série com o resistor desconhecido.
- O Pico lê a tensão no meio do divisor usando seu ADC interno.
- A resistência do resistor desconhecido Rx é calculada usando a fórmula:

$$
R_x = R_{conhecido} \times \frac{ADC_{valor}}{ADC_{resolução} - ADC_{valor}}
$$

## ​

# Como Compilar e testar

- Instale o SDK do Raspberry Pi Pico.
- Instale a biblioteca Raspiberry PI pico no VScode
- Importe o projeto usando a biblioteca.
- Compile o código utilizando o CMake.
