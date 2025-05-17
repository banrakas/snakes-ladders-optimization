#pragma once

#include <stdint.h>
#include <stdlib.h>

struct board {
    size_t size;
    uint32_t faces_in_dice;
    size_t edgelen;
    uint32_t edges[][2];
};

#define OUTPUT

void create_matrix2(size_t edge_len, const uint32_t edges[edge_len][2],
    uint32_t faces_in_dice, size_t dim, float matrix[dim][dim]);

void gaussian_elimination(size_t dim, float matrix[dim][dim], 
    float y[dim], OUTPUT float out[dim]);

float mean_time_theoretical(const size_t boardsize, const uint32_t faces_in_dice, 
    const size_t edgelen, const uint32_t edges[edgelen][2]);