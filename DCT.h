#ifndef DCT_H
#define DCT_H

#include <math.h>
#include <stdlib.h>
#include <stdio.h>

static double M[8][8];

int init_DCT()
{
    for (size_t i = 0; i < 8; i++) {
        M[0][i] = 1/sqrt(8);
    }
    for (size_t i = 0; i < 8; i++) {
        for (size_t j = 0; j < 8; j++) {
            M[i][j] = (i != 0) ? 1./2.*cos((i+i*2*j)/16.*M_PI) : 1/sqrt(8);
        }
    }
    
    return 0;
}

double **matrix_transpose(int m, int n, double A[][m])
{
    double **B_out = (double**)malloc(sizeof(double*)*n);

    for (size_t i = 0; i < n; i++)
        B_out[i] = (double*)malloc(sizeof(double)*m);

    for (size_t i = 0; i < n; i++)
        for (size_t j = 0; j < m; j++)
            B_out[i][j] = A[j][i];

    return B_out;
}

double vector_mult(int n, double *v1, double *v2)
{
    double r = 0;
    for (size_t i = 0; i < n; i++)
        r += v1[i] * v2[i];

    return r;
}

double **matrix_mult(int ma, int na, double A[][ma], int mb, int nb, double B[][mb])
{
    /* Check if multiplication is possible */
    if (na != mb || ma != nb) 
        return 0;

    /* Init C matrix */
    int nc = na;
    int mc = mb;
    double **C_out = (double**)malloc(sizeof(double*)*nc);
    for (size_t i = 0; i < nc; i++)
        C_out[i] = (double*)malloc(sizeof(double)*mc);
    
    for (size_t i = 0; i < nc; i++) {
        double *v1 = A[i]; // v1 size == ma
        for (size_t j = 0; j < mc; j++) {
            double v2[nb];
            for (size_t k = 0; k < nb; k++)
                v2[k] = B[k][j];
            
            C_out[i][j] = vector_mult(ma, v1, v2);
        }
    }
    
    return C_out;
}

double *DCT_1dim(double *V_in, int N)
{
    double *T_out = (double*)malloc(sizeof(double)*N);
    double c;
    
    for (size_t i = 0; i < N; i++) {
        c = (i == 0) ? sqrt((double)1/N) : sqrt((double)2/N);
        T_out[i] = 0;
        for (size_t j = 0; j < N; j++) {
            T_out[i] += c*V_in[j]*cos((2*j+1)*i*M_PI/(2*N));
        }
    }
    
    return T_out;
}

double *IDCT_1dim(double *T_in, int N)
{
    double *V_out = (double*)malloc(sizeof(double)*N);
    double c;
    for (size_t i = 0; i < N; i++) {
        V_out[i] = 0;
        for (size_t j = 0; j < N; j++) {
            c = (j == 0) ? sqrt((double)1/N) : sqrt((double)2/N);
            V_out[i] += c*T_in[j]*cos((2*i+1)*j*M_PI/(2*N));
        }
    }
    
    return V_out;
}

#endif