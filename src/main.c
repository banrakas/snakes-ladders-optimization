#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <assert.h>

#include "boardtime.h"
#include "solver.h"

#define ARRAY_LENGTH(array) (sizeof(array) / sizeof(array[0]))
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

int main(int argc, char** argv)
{
    srand(time(0));
    
    const float time = 16.0;
    const size_t edgelen = 4;
    const size_t boardsize = 25;
    const uint32_t faces_in_dice = 6;
    uint32_t (*edges)[2] = malloc(sizeof(uint32_t[2]) * edgelen);

    optimize_board(boardsize, faces_in_dice, edgelen, time, edges);
    
    const float t = mean_time_theoretical(boardsize, faces_in_dice, edgelen, edges);
    printf("Uh.... we got %f", t);

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

