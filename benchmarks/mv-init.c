#include <stdio.h>

#ifdef TEST
#include "test.h"
#endif

#define M 1024
#define N 1024

double A[M][N];
double B[N];
double C[M];

void init_array()
{
    int i, j;

    for (i = 0; i < M; i++)
        for (j = 0; j < N; j++)
            A[i][j] = (double)(i*j);
    
    for (i = 0; i < N; i++)
        B[i] = (double)(i+j);
    
    for (i = 0; i < M; i++)
        C[i] = (double)(i-j);
}

void print_array()
{
    int i;

    for (i = 0; i < M; i++) {
        fprintf(stderr, "%lf ", C[i]);
        if (i % 20 == 19)
            fprintf(stderr, "\n");
    }
    fprintf(stderr, "\n");
}

void kernel_mv()
{
    int i, j;

#pragma scop
    for (i = 0; i < M; i++) {
        C[i] = 0.0;
        for (j = 0; j < N; j++)
            C[i] = C[i] + A[i][j] * B[j];
    }
#pragma endscop
}

int main()
{
    init_array();
    kernel_mv();
    print_array();

    return 0;
}