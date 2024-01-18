#include "jfif_dump.h"

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("No filename argument\n");
        exit(1);
    }
    char *filename = argv[1];
    handle_loop(filename);

    return 0;
}

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