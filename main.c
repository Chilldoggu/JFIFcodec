// #include "jfif_dump.h"
#include "DCT.h"
#include <stdlib.h>
#include <stdio.h>

#define DEBUG(txt) printf(txt"\n")

int main()
{
    init_DCT();

    int N = 8;
    double V_in[] = {128, 88, 40, 0, 0, 40, 88, 128};

    double A[2][3] = {
        {1, 0, 2}, 
        {-1, 3, 1}
    };

    double B[3][2] = {
        {3, 1},
        {2, 1},
        {1, 0}
    };

    double **C_out = matrix_mult(3, 2, A, 2, 3, B);
    double C[2][2];
    for (size_t i = 0; i < 2; i++)
        for (size_t j = 0; j < 2; j++)
            C[i][j] = C_out[i][j];
    
    double **C_T   = matrix_transpose(2, 2, C);

    for (size_t i = 0; i < 2; i++) {
        for (size_t j = 0; j < 2; j++) {
            printf("%f ", C_out[i][j]);
        }
        printf("\n");
    }

    for (size_t i = 0; i < 2; i++) {
        for (size_t j = 0; j < 2; j++) {
            printf("%f ", C_T[i][j]);
        }
        printf("\n");
    }

    double *T_out = DCT_1dim(V_in, N);

    for (size_t i = 0; i < N; i++)
        printf("%f ", T_out[i]);
    printf("\n");

    double *V_out = IDCT_1dim(T_out, N);

    for (size_t i = 0; i < N; i++) {
        printf("%f ", V_out[i]);
    } printf("\n");

    free(T_out);
    free(V_out);

    return 0;
}

// int main(int argc, char *argv[])
// {
//     if (argc < 2) {
//         printf("No filename argument\n");
//         exit(1);
//     }
//     char *filename = argv[1];
//     handle_loop(filename);
// 
//     return 0;
// }

/*
===== TRASH =====

int main(int argc, char *argv[])
{
    char *input;
    if (argc > 1) {
        input = argv[1];
    } else {
        input = "A MAN A PLAN A CANAL PANAMA";
    }

    char *f_out = "encode";
    int input_size = 0;

    while (input[input_size++] != '\0');

    encode(f_out, input, input_size);
    char *decompressed_data = decode(f_out);

    free(decompressed_data);
    return 0;
}

*/