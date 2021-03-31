#include <stdio.h>

#ifdef TEST
#include "test.h"
#else
#include "common.h"
#endif

#define N 4
#define H 112
#define W 112
#define R 2
#define S 2
#define M 128
#define E 56
#define F 56
#define U 2

double Ifmap[N][M][H][W];
double Ofmap[N][M][E][F];

void init_array()
{
    int i, j, k, m;

    for (i = 0; i < N; i++)
        for (j = 0; j < M; j++)
            for (k = 0; k < H; k++)
                for (m = 0; m < W; m++)
                    Ifmap[i][j][k][m] = (double)(i+k+j+m);
    
    for (i = 0; i < N; i++)
        for (j = 0; j < M; j++)
            for (k = 0; k < E; k++)
                for (m = 0; m < F; m++)
                    Ofmap[i][j][k][m] = 0.0;
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

void kernel_pooling()
{
    int i, j, k, m, n, p, q;

#pragma scop
    for (i = 0; i < N; i++)
        for (j = 0; j < M; j++)
            for (k = 0; k < E; k++)
                for (m = 0; m < F; m++) {
                    Ofmap[i][j][k][m] = Ifmap[i][j][k*U][m*U];
                    for (p = 0; p < R; p++)
                        for (q = 0; q < S; q++)
                            Ofmap[i][j][k][m] = max(Ofmap[i][j][k][m],
                                Ifmap[i][j][k*U+p][m*U+q]);
                }
#pragma endscop
}

int main()
{
    init_array();
    kernel_pooling();
    print_array();

    return 0;
}