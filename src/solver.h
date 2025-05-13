#pragma once

#include <stdlib.h>
#include <stdint.h>

#define OUTPUT

#define max(a, b) (a > b ? a : b)
#define min(a, b) (a > b ? b : a)

// Give edgelen to be even, we want equal no. of snakes and ladders
void board_initialize_random(size_t boardsize, size_t edgelen, OUTPUT uint32_t edges[edgelen][2]);