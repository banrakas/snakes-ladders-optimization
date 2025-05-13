#include "boardtime.h"

#include <string.h>

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
