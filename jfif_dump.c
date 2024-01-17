#include <stdio.h>
#include <stdlib.h>
#include "jfif_dump.h"

// SOSObj SOS_list;
// DHTObj DHT_list;

int main(int argc, char *argv[])
{
    if (argc == 1) {
        printf("No filename argument.\n");
        return 1;
    }

    SOS_list = create_list();
    DHT_list = create_list();
    SOS_list->head = NULL;
    SOS_list->size = 0;
    DHT_list->head = NULL;
    DHT_list->size = 0;

    int r, data_pos;
    unsigned char byte;
    char *filename = argv[1];
    FILE *fp_jfif = fopen(filename, "rb");
    data_pos = 0;

    while (1) { // Loop till EOI
        byte = fgetc(fp_jfif);
        if (byte == 0xFF) {
            if (compressed_data_len) compressed_data_len -= 2;
            r = handle_marker(fp_jfif, fgetc(fp_jfif), &data_pos);
            if (r == EOF || r == 1) break;
        } else if (compressed_data) { 
            compressed_data[data_pos++] = byte;
        }
    }

    #if DEBUG_OUT == 1
    printb(compressed_data, compressed_data_len, 0, 64);
    #endif

    fclose(fp_jfif);
    return 0;
}