#include "jfif_dump.h"
#include "DCT.h"
#include <stdlib.h>
#include <stdio.h>

#define DEBUG(txt) printf(txt"\n")

int main(int argc, char*argv[])
{
    char *filename = argv[1];
    // handle_loop(filename);
    init_DCT();
    zigzag_decode_q("");

    // Sample Y component
    int V_reference[8][8] = {
        {58,  45,  29,  27, 24, 19, 17, 20},
        {62,  52,  42,  41, 38, 30, 22, 18},
        {48,  47,  49,  44, 40, 36, 31, 25},
        {59,  78,  49,  32, 28, 31, 31, 31},
        {98,  138, 116, 78, 39, 24, 25, 27},
        {115, 160, 143, 97, 48, 27, 24, 21},
        {99,  137, 127, 84, 42, 25, 24, 20},
        {74,  95,  82,  67, 40, 25, 25, 19}
    };

    unsigned char Q_Y_ref[64] = {16, 11, 10, 16, 24,  40,  51,  61, 12, 12, 14, 19, 26,  58,  60,  55 ,14, 13, 16, 24, 40,  57,  69,  56 ,14, 17, 22, 29, 51,  87,  80,  62 ,18, 22, 37, 56, 68,  109, 103, 77 ,24, 35, 55, 64, 81,  104, 113, 92, 49, 64, 78, 87, 103, 121, 120, 101 ,72, 92, 95, 98, 112, 100, 103, 99};

    double **V_in = (double**)malloc((sizeof(double*)*8));
    for (size_t i = 0; i < 8; i++) {
        V_in[i] = (double*)malloc((sizeof(double)*8));
        for (size_t j = 0; j < 8; j++)
            V_in[i][j] = V_reference[i][j];
    }
    
    /* Get DCT coefficients after quantization */
    double **T = DCT_2dim((double**)V_in, Q_Y_ref);

    for (size_t i = 0; i < 8; i++)
        for (size_t j = 0; j < 8; j++)
            printf("%4.0f%c", T[i][j], (j == 7) ? '\n' : ' ');
    printf("\n");

    /* Get Y component from DCT coefficients after dequantization */
    double **V = IDCT_2dim(T, Q_Y_ref);

    for (size_t i = 0; i < 8; i++)
        for (size_t j = 0; j < 8; j++)
            printf("%4.0f%c", V[i][j], (j == 7) ? '\n' : ' ');
    
    /* Clear */
    FREE_MATRIX(T, 8);
    FREE_MATRIX(V, 8);
    FREE_MATRIX(V_in, 8);

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