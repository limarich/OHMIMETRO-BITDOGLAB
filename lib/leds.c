#include "leds.h"

// Rotina para definição da intensidade de cores do LED
uint32_t matrix_rgb(uint r, uint g, uint b, float intensity)
{
    uint8_t R = (uint8_t)(r * intensity);
    uint8_t G = (uint8_t)(g * intensity);
    uint8_t B = (uint8_t)(b * intensity);
    return (G << 24) | (R << 16) | (B << 8);
}

// Rotina para acionar a matriz de LEDs - WS2812B
void draw_pio(pixel *draw, PIO pio, uint sm, float intensity)
{
    for (int16_t i = 0; i < PIXELS; i++)
    {
        uint32_t valor_led = matrix_rgb(draw[i].red, draw[i].green, draw[i].blue, intensity);
        pio_sm_put_blocking(pio, sm, valor_led);
    }
}

void test_matrix(PIO pio, uint sm)
{
    frame test_frame, black_frame;

    pixel red = {255, 0, 0}, black = {0, 0, 0};

    for (int16_t i = 0; i < PIXELS; i++)
    {
        test_frame[i] = red;
        black_frame[i] = black;
        draw_pio(test_frame, pio, sm, 0.5);
        sleep_ms(50);
    }
    draw_pio(black_frame, pio, sm, 1);
    sleep_ms(50);
}

// função auxiliar que rotaciona um frame
void rotate_frame(frame *current_frame, uint rotations)
{
    uint SIZE = 5; // numero de linhas/colunas da matriz

    rotations = rotations % 4; // entre 0 e 3 (90˚, 180˚, 270˚)

    frame temp; // copia a matriz original
    for (int i = 0; i < 25; i++)
    {
        temp[i] = (*current_frame)[i];
    }

    for (uint current_rotation = 0; current_rotation < rotations; current_rotation++)
    {
        frame result;
        for (uint row = 0; row < 5; row++)
        {
            for (uint col = 0; col < 5; col++)
            {
                result[col * SIZE + (SIZE - 1 - row)] = temp[row * SIZE + col];
            }
        }
        // Copia de volta para temp para próxima rotação
        for (int i = 0; i < 25; i++)
        {
            temp[i] = result[i];
        }
    }
    // manipula o valor do ponteiro para receber a rotação desejada
    for (int i = 0; i < 25; i++)
    {
        (*current_frame)[i] = temp[i];
    }
}

void draw_arrow(PIO pio, uint sm, direction dir, color_options color)
{
    pixel pixel_color = {255, 0, 0}, black = {0, 0, 0};
    float intensity = 0.125;

    switch (color)
    {
    case RED:
        pixel_color.red = 255;
        pixel_color.green = 0;
        pixel_color.blue = 0;
        break;
    case GREEN:
        pixel_color.red = 0;
        pixel_color.green = 255;
        pixel_color.blue = 0;
        break;
    case BLUE:
        pixel_color.red = 0;
        pixel_color.green = 0;
        pixel_color.blue = 255;
        break;
    case WHITE:
        pixel_color.red = 255;
        pixel_color.green = 255;
        pixel_color.blue = 255;
        break;
    default:
        break;
    }

    frame diagonal_arrow_right_frame = {
        black, black, black, black, pixel_color,
        black, pixel_color, black, black, black,
        pixel_color, black, pixel_color, black, black,
        black, black, black, pixel_color, pixel_color,
        pixel_color, pixel_color, pixel_color, black, black};

    frame diagonal_arrow_left_frame = {
        pixel_color, black, black, black, black,
        black, black, black, pixel_color, black,
        black, black, pixel_color, black, pixel_color,
        pixel_color, pixel_color, black, black, black,
        black, black, pixel_color, pixel_color, pixel_color};

    frame regular_arrow_frame = {
        black,
        black,
        pixel_color,
        black,
        black,
        black,
        black,
        pixel_color,
        black,
        black,
        pixel_color,
        black,
        pixel_color,
        black,
        pixel_color,
        black,
        pixel_color,
        pixel_color,
        pixel_color,
        black,
        black,
        black,
        pixel_color,
        black,
        black,
    };

    if (dir == NORTH || dir == SOUTH || dir == EAST || dir == WEST)
    {
        switch (dir)
        {
        case NORTH:
            break;
        case SOUTH:
            rotate_frame(&regular_arrow_frame, 2);
            break;
        case EAST:
            rotate_frame(&regular_arrow_frame, 3);
            break;
        case WEST:
            rotate_frame(&regular_arrow_frame, 1);
            break;
        default:
            break;
        }
        draw_pio(regular_arrow_frame, pio, sm, intensity);
    }
    else
    {

        switch (dir)
        {
        case SOUTHWEST:
            rotate_frame(&diagonal_arrow_right_frame, 2);
            break;
        case SOUTHEAST:
            rotate_frame(&diagonal_arrow_left_frame, 2);
            break;
        default:
            break;
        }

        if (dir == SOUTHEAST || dir == NORTHWEST)
        {
            draw_pio(diagonal_arrow_left_frame, pio, sm, intensity);
        }
        else
        {
            draw_pio(diagonal_arrow_right_frame, pio, sm, intensity);
        }
    }
}

pixel handle_color(color_options color)
{
    pixel pixel_color = {0, 0, 0};
    switch (color)
    {
    case BLACK:
        pixel_color.red = 0;
        pixel_color.green = 0;
        pixel_color.blue = 0;
        break;
    case BROWN:
        pixel_color.red = 165;
        pixel_color.green = 25;
        pixel_color.blue = 0;
        break;
    case RED:
        pixel_color.red = 255;
        pixel_color.green = 0;
        pixel_color.blue = 0;
        break;
    case ORANGE:
        pixel_color.red = 205;
        pixel_color.green = 43;
        pixel_color.blue = 0;
        break;
    case YELLOW:
        pixel_color.red = 255;
        pixel_color.green = 255;
        pixel_color.blue = 0;
        break;
    case GREEN:
        pixel_color.red = 0;
        pixel_color.green = 255;
        pixel_color.blue = 0;
        break;
    case BLUE:
        pixel_color.red = 0;
        pixel_color.green = 0;
        pixel_color.blue = 255;
        break;
    case VIOLET:
        pixel_color.red = 121;
        pixel_color.green = 8;
        pixel_color.blue = 205;
        break;
    case GRAY:
        pixel_color.red = 55;
        pixel_color.green = 55;
        pixel_color.blue = 55;
        break;
    case WHITE:
        pixel_color.red = 255;
        pixel_color.green = 255;
        pixel_color.blue = 255;
        break;
    default:
        break;
    }

    return pixel_color;
}

void draw_line(PIO pio, uint sm, color_options colors[3])
{
    pixel pixel_color[3];
    frame matrix;
    float intensity = 0.05f;

    for (int i = 0; i < 3; i++) {
        pixel_color[i] = handle_color(colors[i]);
    }

    for (int row = 0; row < 5; row++) {
        for (int col = 0; col < 5; col++) {
            int idx = row * 5 + col;
            if (row < 3) {
                matrix[idx] = pixel_color[row];
            } else {
                matrix[idx] = (pixel){ .red = 0, .green = 0, .blue = 0 };
            }
        }
    }

    draw_pio(matrix, pio, sm, intensity);
}
