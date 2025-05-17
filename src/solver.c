#include "solver.h"

#include <stdbool.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "boardtime.h"

float TIME = 0;
uint32_t faces_in_dice = 0;
size_t boardsize = 0;

/*
TODO:   eliminate global variables
        implement specialized amoeba for this problem
 */

static float conditional_reflect(size_t dim, uint32_t simplex[dim + 1][dim], 
    float y[dim + 1], float (*fn)(const size_t, const uint32_t []), size_t idx, float frac);

static size_t amoeba(size_t dim, uint32_t simplex[dim + 1][dim], 
    float (*fn)(const size_t, const uint32_t []), float ftol, size_t max_iterations);

static float delta_time(const size_t dim, const uint32_t x[dim]);

void optimize_board(size_t bsize, uint32_t faces_dice, size_t edgelen, 
    float time, OUTPUT uint32_t edges[edgelen][2])
{
    boardsize = bsize;
    faces_in_dice = faces_dice;
    TIME = time;

    const size_t dim = edgelen * 2;
    uint32_t (*simplex)[dim] = malloc(sizeof(uint32_t[dim]) * (dim + 1));

    // TODO: Check for degenceracy of simplex
    for (size_t i = 0; i < dim + 1; i++) {
        board_initialize_random(bsize, edgelen, edges);
        
        for (size_t j = 0; j < edgelen; ++j) {
            simplex[i][2 * j]     = edges[j][0];
            simplex[i][2 * j + 1] = edges[j][1];
        }
    }

    size_t lo = amoeba(dim, simplex, &delta_time, 2.0, 6);
    for (size_t i = 0; i < edgelen; ++i) {
        edges[i][0] = simplex[lo][2 * i];
        edges[i][1] = simplex[lo][2 * i + 1];
    }

    free(simplex);
}

//TODO: edges shouldn't end on the same row or col
// Try to edgelen even..?
void board_initialize_random(size_t boardsize, size_t edgelen, 
    OUTPUT uint32_t edges[edgelen][2])
{
    const size_t boarddim = (size_t)floorf(sqrtf(boardsize));
    for (size_t i = 0; i < edgelen; ++i) {
        uint32_t a;
        uint32_t b;
        bool exists;

        // TODO: Maybeee find more efficient ways to do this?
        do {
            a = (rand() % (boardsize - 2)) + 1;
            b = (rand() % (boardsize - 2)) + 1;
            exists = false;
            
            if ((a / boarddim) == (b / boarddim)) {
                exists = true;
                continue;
            }

            // TODO: Add constraints
            for (size_t j = 0; j < i; j++) {
                if (b == edges[j][0] || b == edges[j][1] || 
                    a == edges[j][0] || a == edges[j][1]) {
                    exists = true;
                    break;
                }
            }
        } while (exists);


        if (i < edgelen / 2) {
            edges[i][0] = max(a, b);
            edges[i][1] = min(a, b);
        } else {
            edges[i][1] = max(a, b);
            edges[i][0] = min(a, b);
        }
    }
}

static bool verify_constraints(const size_t boardsize, const size_t dim, 
    const uint32_t x[dim], const struct constraints con)
{
    if (dim % 2 != 0) {
        fputs("Error in verify_constraints(), solver.c: give even dimension", stderr);
        abort();
    }

    const size_t width = (size_t)floorf(sqrt(boardsize));

    for (size_t i = 0; i < dim; i += 2) {
        if (x[i] / width == x[i + 1] / width)
            return false;

        for (size_t j = 0; j < i; j += 2) {
            if (abs(x[i] - x[j]) < con.d1 
            || abs(x[i] - x[j + 1]) < con.d1
            || abs(x[i + 1] - x[j]) < con.d1
            || abs(x[i + 1] - x[j + 1]) < con.d1) {
                return false;
            }
        }
    }

    return true;
}

static bool verify_constraints2(const size_t boardsize, const size_t edgelen, 
    const uint32_t edges[edgelen][2], const struct constraints con)
{
    const size_t width = (size_t)floorf(sqrt(boardsize));

    for (size_t i = 0; i < edgelen; i++) {
        if (edges[i][0] / width == edges[i][1] / width)
            return false;

        for (size_t j = 0; j < i; j++) {
            if (abs(edges[i][0] - edges[j][0]) < con.d1 
            || abs(edges[i][0] - edges[j][1]) < con.d1
            || abs(edges[i][1] - edges[j][0]) < con.d1
            || abs(edges[i][1] - edges[j][1]) < con.d1) {
                return false;
            }
        }
    }

    return true;
}

#define TINY 1.0e-10

static size_t amoeba(size_t dim, uint32_t simplex[dim + 1][dim], 
    float (*fn)(const size_t, const uint32_t []), float ftol, size_t max_iterations)
{
    float *y = malloc(sizeof(float) * (dim + 1));
    size_t hi, hi2, lo = 0;

    for (size_t i = 0; i < dim + 1; ++i) y[i] = fn(dim, simplex[i]);

    while (max_iterations) {
        max_iterations--;
        if (y[0] >= y[1]) {
            hi = 0;
            hi2 = 1;
        } else {
            hi = 1;
            hi2 = 0;
        }

        for (size_t i = 0; i < dim + 1; ++i) {
            if (y[i] > y[hi]) {
                hi2 = hi;
                hi = i;
            } else if (y[i] > y[hi2] && i != hi) {
                hi2 = i;
            }

            if (y[i] < y[lo])
                lo = i;
        }

        // DEBUG PURPOSES
        printf("Current best guess is: ");
        for (size_t i = 0; i < dim; i++) {
            printf("%d ", simplex[lo][i]);
        }
        printf("\n\tCurrent score: %f\n\n", (*fn)(dim, simplex[lo]));

        //TODO: ftol

        float ytry = conditional_reflect(dim, simplex, y, fn, hi, -1.0);

        if (ytry <= y[lo]) {
            ytry = conditional_reflect(dim, simplex, y, fn, hi, 2.0);
        } else if (ytry >= y[hi2]) {
            float ysave = y[hi];

            ytry = conditional_reflect(dim, simplex, y, fn, hi, 0.5);

            if (ytry >= ysave) {
                for (size_t i = 0; i < dim + 1; ++i) {
                    if (i == lo) continue;

                    for (size_t j = 0; j < dim; ++j) {
                        if (abs(simplex[i][j] - simplex[lo][j]) > 1) // To avoid degeneracy
                            simplex[i][j] = 0.5*(simplex[i][j] + simplex[lo][j]);
                    }
                    y[i] = (*fn)(dim, simplex[i]);
                }
            }
        }
    }

    free(y);
    return lo;
}

static size_t amoeba2(size_t edgelen, uint32_t simplex[edgelen + 1][edgelen][2])
{

}

static float conditional_reflect(size_t dim, uint32_t simplex[dim + 1][dim], 
    float y[dim + 1], float (*fn)(const size_t, const uint32_t []), size_t idx, float frac)
{
    float frac1 = (1 - frac)/dim;
    float frac2 = frac1 - frac;

    uint32_t *psum = malloc(sizeof(uint32_t) * dim);
    for (size_t i = 0; i < dim; ++i) {
        psum[i] = 0;
        for (size_t j = 0; j < dim + 1; ++j)
            psum[i] += simplex[j][i];
    }

    uint32_t *x = malloc(sizeof(uint32_t) * dim);
    for (size_t i = 0; i < dim; ++i)
        x[i] = floorf(frac1 * psum[i] - frac2 * simplex[idx][i]);

    float ytry = (*fn)(dim, x);
    if (ytry < y[idx]) {
        y[idx] = ytry;
        for (size_t i = 0; i < dim; ++i)
            simplex[idx][i] = x[i];
    }

    free(x);
    free(psum);

    return ytry;
}

static float delta_time(const size_t dim, const uint32_t x[dim])
{    
    if (dim % 2 != 0) {
        fputs("Error in time(dim, x[dim]), solver.c: give even dimension", stderr);
        abort();
    }

    size_t edgelen = dim / 2;
    uint32_t (*edges)[2] = malloc(sizeof(uint32_t[2]) * edgelen);

    for (size_t i = 0; i < dim / 2; i++) {
        edges[i][0] = x[2 * i];
        edges[i][1] = x[2 * i + 1];
    }

    return fabsf(TIME - mean_time_theoretical(boardsize, faces_in_dice, edgelen, edges));
}
