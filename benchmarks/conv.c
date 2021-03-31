#include <stdio.h>

#ifdef TEST
#include "test.h"
#endif

#define N 1
#define H 6
#define W 6
#define R 3
#define S 3
#define C 4
#define M 4
#define E 4
#define F 4

double Ifmap[N][C][H][W];
double Ofmap[N][M][E][F];
double Filter[M][C][R][S];

void init_array()
{
    int i, j, k, m;
    
    for (i = 0; i < N; i++)
        for (j = 0; j < C; j++)
            for (k = 0; k < H; k++)
                for (m = 0; m < W; m++)
                    Ifmap[i][j][k][m] = (double)(i+j+k+m);
    
    for (i = 0; i < N; i++)
        for (j = 0; j < M; j++)
            for (k = 0; k < E; k++)
                for (m = 0; m < F; m++)
                    Ofmap[i][j][k][m] = 0.0;

    for (i = 0; i < M; i++)
        for (j = 0; j < C; j++)
            for (k = 0; k < R; k++)
                for (m = 0; m < S; m++)
                    Filter[i][j][k][m] = (double)(i+j+k+m);
}

void print_array()
{
    int i, j, k, m;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            for (k = 0; k < E; k++) {
                for (m = 0; m < F; m++) {
                    fprintf(stderr, "%lf ", Ofmap[i][j][k][m]);
                    if (m % 20 == 19)
                        fprintf(stderr, "\n");
                }
                fprintf(stderr, "\n");
            }
            fprintf(stderr, "\n");
        }
        fprintf(stderr, "\n");
    }
    fprintf(stderr, "\n");
}

void kernel_conv()
{
    int i, j, k, m, n, p, q;

#pragma scop
    for (i = 0; i < N; i++)
        for (j = 0; j < M; j++)
            for (k = 0; k < E; k++)
                for (m = 0; m < F; m++)
                    for (n = 0; n < C; n ++)
                        for (p = 0; p < R; p++)
                            for (q = 0; q < S; q++)
                                Ofmap[i][j][k][m] = Ofmap[i][j][k][m] + 
                                    Filter[j][n][p][q] * Ifmap[i][n][k+p][m+q];
#pragma endscop
}

int main()
{
    init_array();
    kernel_conv();
    print_array();

    return 0;
}