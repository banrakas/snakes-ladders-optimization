#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <assert.h>

#define ARRAY_LENGTH(array) (sizeof(array) / sizeof(array[0]))


// void play_move(uint32_t* pos, size_t board_size);
uint32_t roll_dice(uint32_t faces_in_dice);

/** returns the number of moves played in the final game */
uint32_t play_game(size_t size, const uint32_t edges[][2], 
    const size_t edge_len, uint32_t faces_in_dice);

void create_matrix2(size_t edge_len, const uint32_t edges[edge_len][2],
    uint32_t faces_in_dice, size_t dim, float matrix[dim][dim]);

void gaussian_elimination(size_t dim, 
    float matrix[dim][dim], float y[dim], float out[dim]);

int main(int argc, char** argv)
{
    const uint32_t edges[][2] = {{1, 6}, {4, 2}};
    const size_t size = 9;
    const uint32_t faces_in_dice = 3;
    const size_t T = 10000;

    uint32_t sum = 0;    

    srand(time(0));

    for (size_t i = 0; i < T; ++i) {
        uint32_t len = play_game(size, edges, ARRAY_LENGTH(edges), faces_in_dice);
        sum += len;

        printf("No. of moves in game %d: %d\n\n", i + 1, len);
    }

    printf("Avg length of games: %f\n", ((float )sum / T));

    float (*matrix)[size][size] = malloc(sizeof(float[size][size]));

    create_matrix2(ARRAY_LENGTH(edges), edges, faces_in_dice, size, *matrix);

    // for (size_t i = 0; i < size; ++i) {
    //     for (size_t j = 0; j < size; j++) {
    //         printf("%f, ", (*matrix)[i][j]);
    //     }
    //     puts("\t]");
    // }
    
    float *y = malloc(sizeof(float) * size);
    for (size_t i = 0; i < size - 1; ++i) y[i] = 1;
    for (size_t i = 0; i < ARRAY_LENGTH(edges); ++i) y[edges[i][0]] = 0;
    y[size - 1] = 0;

    // for (size_t i = 0; i < size; ++i)
    //     printf("%f ", y[i]);

    // puts("THATS Y");
    
    float *x = malloc(sizeof(float) * size);

    gaussian_elimination(size, *matrix, y, x);
    printf("Theoretical mean length of games is: %f\n", x[0]);

    for (size_t i = 0; i < size; ++i)
        printf("%f ", x[i]);

    puts("THATS X");

    free(matrix);
    free(y);
    free(x);

    return 0;
}

uint32_t play_game(size_t size, const uint32_t edges[][2], 
    const size_t edge_len, uint32_t faces_in_dice)
{
    uint32_t moves = 0;
    uint32_t pos = 0;

    puts("Starting game...");

    while (pos != size - 1) {
        const uint32_t x = roll_dice(faces_in_dice);
        moves++;

        printf("\tPOSITION: %d\tDICE: %d\n", pos, x);
        if (pos + x < size)
            pos += x;
        else
            continue;

        for (size_t i = 0; i < edge_len; ++i) {
            if (pos == edges[i][0]) {
                pos = edges[i][1];         // we will have to ensure there are no nodes which have both an edge going into them and emanating from them
                printf("\tencountered ladder/snake: %d ---> %d\n", edges[i][0], edges[i][1]);
                break;
            }
        }
    }

    return moves;
}

inline uint32_t roll_dice(uint32_t faces_in_dice) 
{
    return (rand() % faces_in_dice) + 1;
}

void create_matrix2(size_t edge_len, const uint32_t edges[edge_len][2],
     uint32_t faces_in_dice, size_t dim, float matrix[dim][dim])
{
    memset(matrix, 0, sizeof(float[dim][dim]));

    for (size_t i = 0; i < edge_len; ++i) {
        matrix[edges[i][0]][edges[i][0]] = 1;
        matrix[edges[i][0]][edges[i][1]] = -1;
    }

    for (size_t row = 0; row < dim - 1; ++row) {
        if (matrix[row][row] != 0) continue;

        if (dim - row > faces_in_dice)
            matrix[row][row] = 1;
        else
            matrix[row][row] = (float)(dim - row -1) / faces_in_dice;

        for (size_t idx = 1; idx <= faces_in_dice && row + idx < dim; ++idx)
            matrix[row][row + idx] = - 1 / (float)faces_in_dice;
    }

    matrix[dim - 1][dim - 1] = 1;
}

// Ax = y
// TODO: Avoids a lot of edge cases, (if the diagonal element is zero)
void gaussian_elimination(size_t dim, 
    float matrix[dim][dim], float y[dim], float out[dim])
{
    float (*gauss)[dim][dim] = malloc(sizeof(float[dim][dim]));

    memcpy(gauss, matrix, sizeof(float[dim][dim]));
    memcpy(out, y, sizeof(float[dim]));

    for (size_t i = 0; i < dim; ++i) {
        float scale = (*gauss)[i][i];
        for (size_t k = i; k < dim && scale != 1; ++k)
            (*gauss)[i][k] /= scale;
        out[i] /= scale;

        for (size_t j = 0; j < dim; ++j) {
            if (j == i || (*gauss)[j][i] == 0) continue;
            scale = (*gauss)[j][i];
            
            for (size_t k = 0; k < dim; ++k)
                (*gauss)[j][k] -= scale * (*gauss)[i][k];
            out[j] -= scale * out[i];
        }
    }

    free(gauss);
}
