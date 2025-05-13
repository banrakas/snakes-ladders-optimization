#include "solver.h"

#include <stdbool.h>
#include <math.h>


//TODO: shouldn't end on the same row or col
void board_initialize_random(size_t boardsize, size_t edgelen, 
    OUTPUT uint32_t edges[edgelen][2])
{
    for (size_t i = 0; i < edgelen; ++i) {
        uint32_t a;
        uint32_t b;
        bool exists;

        // TODO: Maybeee find more efficient ways to do this?
        do {
            a = (rand() % (boardsize - 2)) + 1;
            b = (rand() % (boardsize - 2)) + 1;
            exists = false;
            
            if (a == b) {
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


