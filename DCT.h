#ifndef DCT_H
#define DCT_H

#include <math.h>
#include <stdlib.h>
#include <stdio.h>

// #define PTRTOARR(ARR, N, M, PTR) do{        \
//         for (size_t i = 0; i < N; i++) {    \
//             for (size_t j = 0; j < M; j++)  \
//                 ARR[i][j] = PTR[i][j];      \
//             free(PTR[i]);                   \
//         }                                   \
//         free(PTR);                          \
//         } while (0);

#define FREE_MATRIX(MATRIX, N) do { \
        for (int i = 0; i < N; ++i) \
            free(MATRIX[i]);        \
        free(MATRIX);               \
        } while (0);                \

static double **M;

int init_DCT()
{
    M = (double**)malloc(sizeof(double*)*8);
    for (size_t i = 0; i < 8; i++) {
        M[i] = (double*)malloc(sizeof(double)*8);
        M[0][i] = 1/sqrt(8);
    }
    for (size_t i = 0; i < 8; i++) {
        for (size_t j = 0; j < 8; j++) {
            M[i][j] = (i != 0) ? 1./2.*cos((i+i*2*j)/16.*M_PI) : 1/sqrt(8);
        }
    }
    
    return 0;
}

double **matrix_transpose(int m, int n, double **A)
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

double **matrix_mult(int ma, int na, double **A, int mb, int nb, double **B)
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

double **DCT_quant(double **DCT_table, unsigned char *q_table)
{
    double **q_dct = (double**)malloc(sizeof(double*)*8);
    for (size_t i = 0; i < 8; i++) {
        q_dct[i] = (double*)malloc(sizeof(double)*8);
        for (size_t j = 0; j < 8; j++)
            DCT_table[i][j] = (int)round(DCT_table[i][j]/q_table[i*8+j]);
    }

    return 0;
}

int **IDCT_quant(double **IDCT_table, unsigned char *q_table)
{
    for (size_t i = 0; i < 8; i++) {
        for (size_t j = 0; j < 8; j++)
            IDCT_table[i][j] *= q_table[i*8+j];
    }
    
    return 0;
}

/* Size is alway 8x8 */
double **DCT_2dim(double **V, unsigned char *Q)
{
    double **M_T = matrix_transpose(8, 8, M);
    double **MV  = matrix_mult(8, 8, M, 8, 8, V);
    double **T   = matrix_mult(8, 8, MV, 8, 8, M_T);
    double **T_rounded = (double**)malloc(sizeof(double*)*8);

    for (size_t i = 0; i < 8; i++) {
        T_rounded[i] = (double*)malloc(sizeof(double)*8);
        for (size_t j = 0; j < 8; j++) {
            T_rounded[i][j] = (double)round(T[i][j]);
        }
    }

    /* Perform quantization */
    DCT_quant(T_rounded, Q);

    /* Clear */
    FREE_MATRIX(T, 8);
    FREE_MATRIX(MV, 8);
    FREE_MATRIX(M_T, 8);

    return T_rounded;
}

/* Size is alway 8x8 */
double **IDCT_2dim(double **T, unsigned char *Q)
{
    /* Perform dequantization */
    IDCT_quant(T, Q);

    double **M_T   = matrix_transpose(8, 8, M);
    double **M_TT  = matrix_mult(8, 8, M_T, 8, 8, T);
    
    double **V   = matrix_mult(8, 8, M_TT, 8, 8, M);
    double **V_rounded = (double**)malloc(sizeof(double*)*8);

    for (size_t i = 0; i < 8; i++) {
        V_rounded[i] = (double*)malloc(sizeof(double)*8);
        for (size_t j = 0; j < 8; j++) {
            V_rounded[i][j] = round(V[i][j]);
        }
    }

    /* Clear */
    FREE_MATRIX(M_T, 8);
    FREE_MATRIX(M_TT, 8);
    FREE_MATRIX(V, 8);

    return V_rounded;
}

unsigned char *zigzag_decode_q(unsigned char *bytes)
{
    int iter = 0, i = 0, j = 0, loop_up = 0, loop_down = 0;
    unsigned char Q_tmp[8][8] = {};
    /* Decode 64 byte table of quantization values into array */
    unsigned char *Q = (unsigned char*)malloc(sizeof(unsigned char)*64);

    int n = 3, m = 2;
    while (iter != n*m) {
        Q_tmp[i][j] = bytes[iter];
        // Q_tmp[i][j] = iter;
        if (!loop_up && !loop_down) {
            if (i == 0 && j < m || i == n-1) {
                j++;
            } else if (j == 0 && i < n || j == m-1) {
                i++;
            }

            if (i == 0 || j == m-1)
                loop_down = 1;
            else if (j == 0 || i == n-1)
                loop_up = 1;            
            iter++;
            continue;
        }

        if (loop_down) {            
            i++;
            j--;
        } else if (loop_up) {
            i--;
            j++;
        }

        if (j == 0 || j == m-1 || i == 0 || i == n-1) {
            loop_down = 0;
            loop_up = 0;
        }

        iter++;
    }

    for (size_t i = 0; i < 8; i++)
        for (size_t j = 0; j < 8; j++) {
            Q[i*8+j] = Q_tmp[i][j];
            // printf("%3d%c", Q_tmp[i][j], (j == 7) ? '\n' : ' ');
        }
    // printf("\n");

    return Q;
}

#endif