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
        maintain constraints during amoeba movement
        maybe implement specialized amoeba for this problem?
 */

static float conditional_reflect(const size_t dim, uint32_t simplex[dim + 1][dim], 
    float y[dim + 1], float (*fn)(const size_t, const uint32_t []), const size_t idx, const float frac);

static size_t amoeba(size_t dim, uint32_t simplex[dim + 1][dim], 
    float (*fn)(const size_t, const uint32_t []), float ftol, size_t max_iterations);

static float delta_time(const size_t dim, const uint32_t x[dim]);

// generate simplex around the x point
static void generate_simplex(size_t bsize, size_t dim, 
    uint32_t x[dim], OUTPUT uint32_t simplex[dim + 1][dim]);


/* Returns true if the board verifies the constraints */
static bool verify_constraints2(const size_t boardsize, const size_t edgelen, 
    const uint32_t edges[edgelen][2], const struct constraints con);

void optimize_board(size_t bsize, uint32_t faces_dice, size_t edgelen, 
    float time, OUTPUT uint32_t edges[edgelen][2])
{
    boardsize = bsize;
    faces_in_dice = faces_dice;
    TIME = time;

    size_t width = (size_t)floorf(sqrtf(bsize));

    const size_t dim = edgelen * 2;
    uint32_t (*simplex)[dim] = malloc(sizeof(uint32_t[dim]) * (dim + 1));

    struct constraints con = {
        .d1 = 2
    };

    board_initialize_random(bsize, edgelen, edges, con);

    for (size_t j = 0; j < edgelen; ++j) {
        simplex[0][2 * j]     = edges[j][0];
        simplex[0][2 * j + 1] = edges[j][1];
    }

    for (size_t i = 1; i < dim + 1; i++) {
        memcpy(simplex[i], simplex[0], sizeof(simplex[0][0]) * dim);

        // TODO: constraints
        if (simplex[i][i - 1] < bsize - width - 1)
            simplex[i][i - 1] += width;
        else
            simplex[i][i - 1] -= width;
    }

    for (size_t i = 0; i < 10; i++) {
        size_t lo = amoeba(dim, simplex, &delta_time, 0.01, 100);
        generate_simplex(bsize, dim, simplex[lo], simplex);
    }
    for (size_t i = 0; i < edgelen; ++i) {
        edges[i][0] = simplex[0][2 * i];
        edges[i][1] = simplex[0][2 * i + 1];
    }

    free(simplex);
}

// Try to give edgelen even..?
void board_initialize_random(size_t boardsize, size_t edgelen, 
    OUTPUT uint32_t edges[edgelen][2], const struct constraints con)
{
    for (size_t i = 0; i < edgelen; ++i) {
        uint32_t a;
        uint32_t b;

        do {
            a = (rand() % (boardsize - 2)) + 1;
            b = (rand() % (boardsize - 2)) + 1;

            if (i < edgelen / 2) {
                edges[i][0] = max(a, b);
                edges[i][1] = min(a, b);
            } else {
                edges[i][1] = max(a, b);
                edges[i][0] = min(a, b);
            }
        } while (!verify_constraints2(boardsize, i + 1, edges, con));
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

// TODO: Add more constraints
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

static void generate_simplex(size_t bsize, size_t dim, 
    uint32_t x[dim], OUTPUT uint32_t simplex[dim + 1][dim])
{
    memcpy(simplex[0], x, sizeof(simplex[0][0]) * dim);
    const size_t width = (size_t) floorf(sqrtf(bsize));

    for (size_t i = 1; i < dim + 1; i++) {
        memcpy(simplex[i], simplex[0], sizeof(simplex[0][0]) * dim);

        // TODO: constraints
        if (simplex[i][i - 1] < bsize - width - 1)
            simplex[i][i - 1] += width;
        else
            simplex[i][i - 1] -= width;
    }
}

#define TINY 1.0e-10

static size_t amoeba(size_t dim, uint32_t simplex[dim + 1][dim], 
    float (*fn)(const size_t, const uint32_t []), float ftol, size_t max_iterations)
{
    float *y = malloc(sizeof(float) * (dim + 1));
    size_t hi, hi2, lo = 0;

    for (size_t i = 0; i < dim + 1; ++i) {
        y[i] = fn(dim, simplex[i]);
        if (y[i] < y[lo]) lo = i; // only for printing purposes
    }

    printf("INTIAL BEST SCORE:\t%f\n", y[lo]);

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
        // printf("Current best guess is #{%ld}: ", max_iterations);
        // for (size_t i = 0; i < dim; i++) {
        //     printf("%d ", simplex[lo][i]);
        // }

        // if (verify_constraints(boardsize, dim, simplex[lo], (struct constraints) {
        //     .d1 = 1,
        // })) {
        //     printf("\nverifies constraints!");
        // }

        // printf("\n\tCurrent score: %f\n\n", (*fn)(dim, simplex[lo]));
        // ---- \debug


        float ysave = y[hi];
        float ytry = conditional_reflect(dim, simplex, y, fn, hi, -1.0);

        if (fabsf(ysave - ytry) < ftol) {
            break;
        }

        if (ytry <= y[lo]) {
            ytry = conditional_reflect(dim, simplex, y, fn, hi, 2.0);
        } else if (ytry >= y[hi2]) {
            ysave = y[hi];

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

    printf("Current best guess is #{%ld}: ", max_iterations);
    for (size_t i = 0; i < dim; i++) {
        printf("%d ", simplex[lo][i]);
    }

    if (verify_constraints(boardsize, dim, simplex[lo], (struct constraints) {
        .d1 = 1,
    })) {
        printf("\nverifies constraints!");
    }

    printf("\nFINAL BEST SCORE:\t%f\n", y[lo]);

    free(y);
    return lo;
}

/* Returns true if x occurs twice in arr */
static bool repeats(const size_t n, const uint32_t arr[n], const uint32_t x)
{
    bool once = false;
    for (size_t i = 0; i < n; ++i) {
        if (arr[i] == x) {
            if (once)
                return true;
            
            once = !once;
        }
    }
    return false;
}

// GLOBAL var used: boardsize
static float conditional_reflect(const size_t dim, uint32_t simplex[dim + 1][dim], 
    float y[dim + 1], float (*fn)(const size_t, const uint32_t []), const size_t idx, const float frac)
{
    const float frac1 = (1 - frac)/dim;
    const float frac2 = frac1 - frac;

    uint32_t *psum = malloc(sizeof(uint32_t) * dim);
    for (size_t i = 0; i < dim; ++i) {
        psum[i] = 0;
        for (size_t j = 0; j < dim + 1; ++j)
            psum[i] += simplex[j][i];
    }

    uint32_t *x = malloc(sizeof(uint32_t) * dim);
    for (size_t i = 0; i < dim; ++i) {
        x[i] = roundf(frac1 * psum[i] - frac2 * simplex[idx][i]);

        /*
        One of the reasons why the simplex converges slowly towards the minimum is
        because the fn function is highly discontinuous. Suppose you were to change the position
        of a snake head by one cell such that it now leads into a ladder. This small change can drastically
        change the value of fn(). To mitigate this, we can either change our amoeba function such that this
        doesn't happen or change our fn(). Let's try both and see which yields better results.

        Nvm fixed it by running amoeba 10 times.
        */
        if (x[i] == 0) x[i] = 1;

        if (repeats(i + 1, x, x[i])) {
            if (!repeats(i + 1, x, x[i] + 1))
                x[i]++;
            else if (!repeats(i + 1, x, x[i] - 1) && x[i] != 1)
                x[i]--;
        }

        while (repeats(i + 1, x, x[i]))
            x[i]++;

        // VERY CRUDE CONSTRAINT HANDLING
        if (x[i] >= boardsize - 1) x[i] = boardsize - 2;

        while (repeats(i + 1, x, x[i]))
            x[i]--;
    }

    float ytry = fn(dim, x);
    if (ytry < y[idx]) {
        y[idx] = ytry;
        for (size_t i = 0; i < dim; ++i)
            simplex[idx][i] = x[i];
    }

    free(x);
    free(psum);

    return ytry;
}


// global var used: TIME
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
