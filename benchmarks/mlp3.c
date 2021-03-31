#include <stdio.h>

#ifdef TEST
#include "test.h"
#else
#include "common.h"
#endif

#define N 16
#define I1 2048
#define O1 4096
#define O2 4096
#define O3 1000

double Input[N][I1];
double Output1[N][O1];
double Output2[N][O2];
double Output3[N][O3];
double Weight1[I1][O1];
double Weight2[O1][O2];
double Weight3[O2][O3];

void init_array()
{
    int i, j;

    for (i = 0; i < N; i++)
        for (j = 0; j < I1; j++)
            Input[i][j] = (double)(i+j);
    
    for (i = 0; i < N; i++)
        for (j = 0; j < O1; j++)
            Output1[i][j] = 0.0;
    
    for (i = 0; i < N; i++)
        for (j = 0; j < O2; j++)
            Output2[i][j] = 0.0;
    
    for (i = 0; i < N; i++)
        for (j = 0; j < O3; j++)
            Output3[i][j] = 0.0;
        
    for (i = 0; i < I1; i++)
        for (j = 0; j < O1; j++)
            Weight1[i][j] = (double)(i+j);
    
    for (i = 0; i < O1; i++)
        for (j = 0; j < O2; j++)
            Weight2[i][j] = (double)(i*j);
    
    for (i = 0; i < O2; i++)
        for (j = 0; j < O3; j++)
            Weight3[i][j] = (double)(i/(j+1));
}

void print_array()
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < O3; j++) {
            fprintf(stderr, "%lf ", Output3[i][j]);
            if (j % 20 == 19)
                fprintf(stderr, "\n");
        }
        fprintf(stderr, "\n");
    }
    fprintf(stderr, "\n");
}

void kernel_mlp3()
{
    int i, j, k;

#pragma scop
    for (i = 0; i < N; i++) {
        for (j = 0; j < O1; j++) {
            for (k = 0; k < I1; k++)
                Output1[i][j] += Weight1[k][j] * Input[i][k];
            Output1[i][j] = ReLU(Output1[i][j]);
        }
    }
    
    for (i = 0; i < N; i++) {
        for (j = 0; j < O2; j++) {
            for (k = 0; k < O1; k++)
                Output2[i][j] += Weight2[k][j] * Output1[i][k];
            Output2[i][j] = ReLU(Output2[i][j]);
        }
    }
    
    for (i = 0; i < N; i++) {
        for (j = 0; j < O3; j++) {
            for (k = 0; k < O2; k++)
                Output3[i][j] += Weight3[k][j] * Output2[i][k];
            Output3[i][j] = ReLU(Output3[i][j]);
        }
    
    }
#pragma endscop
}

int main()
{
    init_array();
    kernel_mlp3();
    print_array();

    return 0;
}