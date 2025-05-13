#pragma once

#include <stdint.h>
#include <stdlib.h>

void create_matrix2(size_t edge_len, const uint32_t edges[edge_len][2],
    uint32_t faces_in_dice, size_t dim, float matrix[dim][dim]);

void gaussian_elimination(size_t dim, 
    float matrix[dim][dim], float y[dim], float out[dim]);