#include <stdio.h>

#ifdef TEST
#include "test.h"
#endif

#define M 1024
#define N 2048
#define K 512

double A[M][K];
double B[K][N];
double C[M][N];

void init_array()
{
    int i, j;

    for (i = 0; i < M; i++)
        for (j = 0; j < K; j++)
            A[i][j] = (double)(i+j);
    
    for (i = 0; i < K; i++)
        for (j = 0; j < N; j++)
            B[i][j] = (double)(i*j);

    for (i = 0; i < M; i++)
        for (j = 0; j < N; j++)
            C[i][j] = 0.0;
}

void print_array()
{
    int i, j;

    for (i = 0; i < M; i++) {
        for (j = 0; j < N; j++) {
            fprintf(stderr, "%lf ", C[i][j]);
            if (j % 20 == 19)
                fprintf(stderr, "\n");
        }
        fprintf(stderr, "\n");
    }
    fprintf(stderr, "\n");
}

void kernel_mm()
{
    int i, j, k;

#pragma scop
    for (i = 0; i < M; i++)
        for (j = 0; j < N; j++)
            for (k = 0; k < K; k++)
                C[i][j] = C[i][j] + A[i][k] * B[k][j];
#pragma endscop
}

int main()
{
    init_array();
    kernel_mm();
    print_array();

    return 0;
}