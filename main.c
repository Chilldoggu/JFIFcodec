#include <stdlib.h>
#include <stdio.h>
#include "jfif_dump.h"
#include "jfif_huffman.h"
#include "DCT.h"

int last_dc = 0;

int main(int argc, char*argv[])
{

    if (argc < 2) {
        printf("No filename argument\n");
        exit(1);
    }
    char *filename = argv[1];
    handle_loop(filename);

    init_DCT();
    printf("%d\n", SOF0.Ncomp);
    if (init_huffman("test", &SOF0)) {
        printf("Error while processing init_huffman().\n");
    }
    
    return 0;
}

/*
===== TRASH =====
{
    GENERIC_MARKER arr[DHT_list->size+1];
    {
        size_t i = 0;
        for (; i < DHT_list->size; i++) {
            DHT *DHT_body = (DHT*)list_at(DHT_list, i);
            arr[i].marker = DHT_body->marker;
            arr[i].body = DHT_body;
        }
        arr[i].marker = DQT.marker;
        arr[i].body = &DQT;
    }

    FILE *fp_tables = fopen("tables.txt", "w");
    print_markers(fp_tables, arr, DHT_list->size+1);
    fclose(fp_tables);
}

{
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
}


{
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
}

*/