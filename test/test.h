#ifndef TEST_H
#define TEST_H

int min(int a, int b);

double max(double a, double b);

double tanh(double in);
double sigmoid(double in);
double ReLU(double in);

void xbblas_mv_bare(int pos, int dm, int dn,
    double *A, int lda, double *B, double *C, int init);
void xbblas_mv_act(int pos, int dm, int dn,
    double *A, int lda, double *B, double *C, double (*act)(double in),
    int init);

void xbblas_mm_bare(int pos, int dm, int dn, int dk,
    double *A, int lda, double *B, int ldb, double *C, int ldc, int init);
void xbblas_mm_act(int pos, int dm, int dn, int dk,
    double *A, int lda, double *B, int ldb, double *C, int ldc,
    double (*act)(double in), int init);

void conv_load_filter(double *src, int oc, int ic, int kh, int kw, double *dst);
void conv_load_data(int dm, int dn, int dk,
    int oc, int oh, int ow, int ic, int kh, int kw,int strd, 
    double *src1, double *dst1, int d10, int d11,
    double *src2, double *dst2, int d20, int d21);
void conv_store_data(int dm, int dn, int dk,
    int oc, int oh, int ow,
    double *src, double *dst, int d0, int d1);

void pool_load_data(int dm, int dn,
    int oc, int oh, int ow, int kh, int kw,int strd, 
    double *src1, double *dst1, int d10, int d11,
    double *src2, double *dst2, int d20, int d21);
void pool_store_data(int dm, int dn,
    int oc, int oh, int ow,
    double *src, double *dst, int d0, int d1);
void xbblas_pooling(int pos, int dm, int dn,
    double *A, int lda, double *B, int ldb, int init);

void xbblas_malloc_3d(double ****A, int d0, int d1, int d2);
void xbblas_malloc_2d(double ***A, int d0, int d1);

void fence();

#endif