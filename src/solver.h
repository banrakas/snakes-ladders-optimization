#pragma once

#include <stdlib.h>
#include <stdint.h>

#define OUTPUT

#define max(a, b) (a > b ? a : b)
#define min(a, b) (a > b ? b : a)

// along with that edges should not end on the same row
struct constraints {
    uint32_t d1;
};

// Give edgelen to be even, we want equal no. of snakes and ladders
void board_initialize_random(size_t boardsize, size_t edgelen, 
    OUTPUT uint32_t edges[edgelen][2], const struct constraints con);

void optimize_board(size_t bsize, uint32_t faces_dice, size_t edgelen, 
    float time, OUTPUT uint32_t edges[edgelen][2]);