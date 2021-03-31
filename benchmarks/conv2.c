#include <stdio.h>

#ifdef TEST
#include "test.h"
#else
#include "common.h"
#endif

#define N 4
#define H1 6
#define W1 6
#define R1 3
#define S1 3
#define C1 4
#define M1 4
#define E1 4
#define F1 4
#define H2 E1
#define W2 F1
#define R2 3
#define S2 3
#define C2 M1
#define M2 4
#define E2 2
#define F2 2

double Ifmap1[N][C1][H1][W1];
double Ofmap1[N][M1][E1][F1];
double Filter1[M1][C1][R1][S1];
double Ofmap2[N][M2][E2][F2];
double Filter2[M2][C2][R2][S2];

void init_array()
{
    int i, j, k, m;
    
    for (i = 0; i < N; i++)
        for (j = 0; j < C1; j++)
            for (k = 0; k < H1; k++)
                for (m = 0; m < W1; m++)
                    Ifmap1[i][j][k][m] = (double)(i+k+j+m);
    
    for (i = 0; i < N; i++)
        for (j = 0; j < M1; j++)
            for (k = 0; k < E1; k++)
                for (m = 0; m < F1; m++)
                    Ofmap1[i][j][k][m] = 0.0;

    for (i = 0; i < M1; i++)
        for (j = 0; j < C1; j++)
            for (k = 0; k < R1; k++)
                for (m = 0; m < S1; m++)
                    Filter1[i][j][k][m] = (double)(i+j+k+m);
    
    for (i = 0; i < N; i++)
        for (j = 0; j < M2; j++)
            for (k = 0; k < E2; k++)
                for (m = 0; m < F2; m++)
                    Ofmap2[i][j][k][m] = 0.0;
    
    for (i = 0; i < M2; i++)
        for (j = 0; j < C2; j++)
            for (k = 0; k < R2; k++)
                for (m = 0; m < S2; m++)
                    Filter2[i][j][k][m] = (double)(i+j+k+m);
}

void print_array()
{
    int i, j, k, m;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M2; j++) {
            for (k = 0; k < E2; k++) {
                for (m = 0; m < F2; m++) {
                    fprintf(stderr, "%lf ", Ofmap2[i][j][k][m]);
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

void kernel_conv2()
{
    int i, j, k, m, n, p, q;

#pragma scop
    for (i = 0; i < N; i++) {

        for (j = 0; j < M1; j++)
            for (k = 0; k < E1; k++)
                for (m = 0; m < F1; m++) {
                    for (n = 0; n < C1; n ++)
                        for (p = 0; p < R1; p++)
                            for (q = 0; q < S1; q++)
                                Ofmap1[i][j][k][m] = Ofmap1[i][j][k][m] + 
                                    Filter1[j][n][p][q] *
                                    Ifmap1[i][n][k+p][m+q];
                    Ofmap1[i][j][k][m] = ReLU(Ofmap1[i][j][k][m]);
                }
    
        for (j = 0; j < M2; j++)
            for (k = 0; k < E2; k++)
                for (m = 0; m < F2; m++)
                    for (n = 0; n < C2; n ++)
                        for (p = 0; p < R2; p++)
                            for (q = 0; q < S2; q++)
                                Ofmap2[i][j][k][m] = Ofmap2[i][j][k][m] + 
                                    Filter2[j][n][p][q] *
                                    Ofmap1[i][n][k+p][m+q];
    }
#pragma endscop
}

int main()
{
    init_array();
    kernel_conv2();
    print_array();

    return 0;
}