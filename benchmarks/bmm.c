#include <stdio.h>

#ifdef TEST
#include "test.h"
#endif

#define P 4
#define M 8
#define N 4
#define K 4

double A[P][M][K];
double B[K][N];
double C[P][M][N];

void init_array() {
    int i, j, k;

    for (k = 0; k < P; k++)
        for (i = 0; i < M; i++)
            for (j = 0; j < K; j++)
                A[k][i][j] = (double)(i+j+k);
    
    for (i = 0; i < K; i++)
        for (j = 0; j < N; j++)
            B[i][j] = (double)(i+j);

    for (k = 0; k < P; k++)
        for (i = 0; i < M; i++)
            for (j = 0; j < N; j++)
                C[k][i][j] = 0.0;
}

void print_array() {
    int i, j, k;

    for (k = 0; k < P; k++) {
        for (i = 0; i < N; i++) {
            for (j = 0; j < N; j++) {
                fprintf(stderr, "%lf ", C[k][i][j]);
                if (j % 20 == 19)
                    fprintf(stderr, "\n");
            }
            fprintf(stderr, "\n");
        }
        fprintf(stderr, "\n");
    }
    fprintf(stderr, "\n");
}

void kernel_bmm()
{
    int i, j, k, m;

#pragma scop
    for (m = 0; m < P; m++)
        for (i = 0; i < M; i++)
            for (j = 0; j < N; j++)
                for (k = 0; k < K; k++)
                    C[m][i][j] = C[m][i][j] + A[m][i][k] * B[k][j];
#pragma endscop
}

int main()
{
    init_array();
    kernel_bmm();
    print_array();

    return 0;
}