#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <assert.h>

#include "boardtime.h"
#include "solver.h"

#define ARRAY_LENGTH(array) (sizeof(array) / sizeof(array[0]))

struct board {
    size_t size;
    uint32_t faces_in_dice;
    size_t edgelen;
    uint32_t edges[][2];
};


// void play_move(uint32_t* pos, size_t board_size);
uint32_t roll_dice(uint32_t faces_in_dice);

/** returns the number of moves played in the final game */
uint32_t play_game(size_t size, const uint32_t edges[][2], 
    const size_t edge_len, uint32_t faces_in_dice);

void mean_time_empirical(const struct board *board, const size_t T)
{
    uint32_t sum = 0;

    for (size_t i = 0; i < T; ++i) {
        uint32_t len = play_game(board->size, board->edges, board->edgelen, board->faces_in_dice);
        sum += len;
    }

    printf("Avg length of games: %f\n", ((float )sum / T));
}

void mean_time_theoretical(const struct board *board)
{
    const size_t size = board->size;
    const size_t edgelen = board->edgelen;

    float (*matrix)[size][size] = malloc(sizeof(float[size][size]));

    create_matrix2(board->edgelen, board->edges, board->faces_in_dice, board->size, *matrix);
    
    float *y = malloc(sizeof(float) * size);
    for (size_t i = 0; i < size - 1; ++i) y[i] = 1;
    for (size_t i = 0; i < edgelen; ++i) y[board->edges[i][0]] = 0;
    y[size - 1] = 0;
    
    float *x = malloc(sizeof(float) * size);

    gaussian_elimination(size, *matrix, y, x);
    printf("Theoretical mean length of games is: %f\n", x[0]);

    free(matrix);
    free(y);
    free(x);
}

int main(int argc, char** argv)
{
    srand(time(0));
    
    const size_t edgelen = 4;
    struct board *board = malloc(sizeof(struct board) + sizeof(board->edges[0]) * edgelen);

    board->edgelen = edgelen;
    board->faces_in_dice = 3;
    board->size = 25;

    board_initialize_random(board->size, edgelen, board->edges);

    for (size_t i = 0; i < edgelen; i++)
        printf("[%d, %d]", board->edges[i][0], board->edges[i][1]);
    puts("");

    mean_time_empirical(board, 1000);
    mean_time_theoretical(board);
    

    free(board);
    return 0;
}

uint32_t play_game(size_t size, const uint32_t edges[][2], 
    const size_t edge_len, uint32_t faces_in_dice)
{
    uint32_t moves = 0;
    uint32_t pos = 0;

    // puts("Starting game...");

    while (pos != size - 1) {
        const uint32_t x = roll_dice(faces_in_dice);
        moves++;

        // printf("\tPOSITION: %d\tDICE: %d\n", pos, x);
        if (pos + x < size)
            pos += x;
        else
            continue;

        for (size_t i = 0; i < edge_len; ++i) {
            if (pos == edges[i][0]) {
                pos = edges[i][1];         // we will have to ensure there are no nodes which have both an edge going into them and emanating from them
                // printf("\tencountered ladder/snake: %d ---> %d\n", edges[i][0], edges[i][1]);
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

