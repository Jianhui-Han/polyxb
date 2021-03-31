#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "test.h"

int min(int a, int b)
{
    if (a >= b)
        return b;
    return a;
}

double max(double a, double b)
{
    if (a >= b)
        return a;
    return b;
}

double tanh(double in)
{
    return (exp(in) - exp(-1 * in)) / (exp(in) + exp(-1 * in));
}

double sigmoid(double in)
{
    return 1 - (1 + exp(-1 * in));
}

double ReLU(double in)
{
    return (in >= 0 ? in : 0);
}

void xbblas_mv_bare(int pos, int dm, int dn,
    double *A, int lda, double *B, double *C, int init)
{
    int i, j;

    for (i=0; i<dm; i++) {
        if (init)
            *(C + i) = 0.0;
        for (j=0; j<dn; j++)
            *(C + i) += *(A + i*lda + j) * *(B + j);
    }
}

void xbblas_mv_act(int pos, int dm, int dn,
    double *A, int lda, double *B, double *C, double (*act)(double in),
    int init)
{
    int i, j;

    for (i=0; i<dm; i++) {
        if (init)
            *(C + i) = 0.0;
        for (j=0; j<dn; j++)
            *(C + i) += *(A + i*lda + j) * *(B + j);
        *(C + i) = act(*(C + i));
    }
}

void xbblas_mm_bare(int pos, int dm, int dn, int dk,
    double *A, int lda, double *B, int ldb, double *C, int ldc, int init)
{
    int i, j, k;

    for (i=0; i<dm; i++)
        for (j=0; j<dn; j++) {
            if (init)
                *(C + i*ldc + j) = 0.0;
            for (k=0; k<dk; k++)
                *(C + i*ldc + j) += *(A + i*lda + k) * *(B + k*ldb + j);
        }
}

void xbblas_mm_act(int pos, int dm, int dn, int dk,
    double *A, int lda, double *B, int ldb, double *C, int ldc,
    double (*act)(double in), int init)
{
    int i, j, k;

    for (i=0; i<dm; i++)
        for (j=0; j<dn; j++) {
            if (init)
                *(C + i*ldc + j) = 0.0;
            for (k=0; k<dk; k++)
                *(C + i*ldc + j) += *(A + i*lda + k) * *(B + k*ldb + j);
            *(C + i*ldc +j) = act(*(C + i*ldc + j));    
        }
}

void conv_load_filter(double *src, int oc, int ic, int kh, int kw, double *dst)
{
    int i, j, k, l;
    int row;
    
    for (j=0; j<ic; j++)
        for (k=0; k<kh; k++)
            for (l=0; l<kw; l++) {
                for (i=0; i<oc; i++){
                    *(dst + j*kh*kw*oc + k*kw*oc + l*oc + i) =
                        *(src + i*kw*kh*ic + j*kw*kh + k*kw + l);
                }
            }
}

void conv_load_data(int dm, int dn, int dk,
    int oc, int oh, int ow, int ic, int kh, int kw,int strd, 
    double *src1, double *dst1, int d10, int d11, double *src2, double *dst2,
    int d20, int d21)
{
    int i, j;
    int a, b, c;
    int x, y, z;
    int ih, iw;

    ih = (oh - 1) * strd + kh;
    iw = (ow - 1) * strd + kw;

    for (i=0; i<dm; i++) {
        a = d10 + i;
        for (j=0; j<dk; j++) {
            b = d11 + j;
            x = b / (kh * kw);
            y = b % (kh * kw) / kw + a / ow * strd;
            z = b % kw + a % ow * strd;
            *(dst1 + i*ic*kh*kw + j) = *(src1 + x*ih*iw + y*iw + z);
        }
    }

    for (i=0; i<dm; i++) {
        b = (i+d20) / ow;
        c = (i+d20) % ow;
        for (j=0; j<dn; j++) {
            a = j+d21;
            *(dst2 + i*oc + j) = *(src2 + a*oh*ow + b*ow + c);
        }
    }
}

void conv_store_data(int dm, int dn, int dk,
    int oc, int oh, int ow,
    double *src, double *dst, int d0, int d1)
{
    int i, j;
    int a, b, c;

    for (i=0; i<dm; i++) {
        b = (i+d0) / ow;
        c = (i+d0) % ow;
        for (j=0; j<dn; j++) {
            a = j+d1;
            *(dst + a*oh*ow + b*ow + c) = *(src + i*oc + j);
        }
    }
}

void pool_load_data(int dm, int dn,
    int oc, int oh, int ow, int kh, int kw,int strd, 
    double *src1, double *dst1, int d10, int d11,
    double *src2, double *dst2, int d20, int d21)
{
    int i, j;
    int a, b, c;
    int x, y, z;
    int ih, iw;

    ih = (oh - 1) * strd + kh;
    iw = (ow - 1) * strd + kw;

    for (i=0; i<dm; i++) {
        a = d10 + i;
        for (j=0; j<dn*kh*kw; j++) {
            b = d11*kh*kw + j;
            x = b / (kh * kw);
            y = b % (kh * kw) / kw + a / ow * strd;
            z = b % kw + a % ow * strd;
            *(dst1 + i*oc*kh*kw + j) = *(src1 + x*ih*iw + y*iw + z);
        }
    }

    for (i=0; i<dm; i++) {
        b = (i+d20) / ow;
        c = (i+d20) % ow;
        for (j=0; j<dn; j++) {
            a = j+d21;
            *(dst2 + i*oc + j) = *(src2 + a*oh*ow + b*ow + c);
        }
    }
}

void pool_store_data(int dm, int dn,
    int oc, int oh, int ow,
    double *src, double *dst, int d0, int d1)
{
    int i, j;
    int a, b, c;

    for (i=0; i<dm; i++) {
        b = (i+d0) / ow;
        c = (i+d0) % ow;
        for (j=0; j<dn; j++) {
            a = j+d1;
            *(dst + a*oh*ow + b*ow + c) = *(src + i*oc + j);
        }
    }
}

void xbblas_pooling(int pos, int dm, int dn,
    double *A, int lda, double *B, int ldb, int init)
{
    int pool = lda / ldb;
    int i, j, k;

    for (i=0; i<dm; i++) {
        for (j=0; j<dn; j++) {
            *(B + i*ldb + j) = 0;
            for (k=0; k<pool; k++) {
                *(B + i*ldb + j) = max(*(B + i*ldb + j), *(A + i*lda + j*pool + k));
            }
        }
    }
}

void xbblas_malloc_3d(double ****A, int d0, int d1, int d2)
{
    int i, j;
    double *temp = (double*)malloc(sizeof(double)*d0*d1*d2);

    *A = (double***)malloc(sizeof(double**) * d0);
    for (i=0; i<d0; i++) {
        (*A)[i] = (double**)malloc(sizeof(double*) * d1);
        for (j=0; j<d1; j++)
            (*A)[i][j] = temp + d2*d1*i + d2*j;
    }
}

void xbblas_malloc_2d(double ***A, int d0, int d1)
{
    int i;
    double *temp = (double*)malloc(sizeof(double)*d0*d1);

    *A = (double**)malloc(sizeof(double*) * d0);
    for (i=0; i<d0; i++)
        (*A)[i] = temp + d1*i;
}

void fence()
{
    return;
}