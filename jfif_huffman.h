#ifndef JFIF_HUFFMAN_H
#define JFIF_HUFFMAN_H

typedef struct mcu {
    int n;
    double *data_units;
} MCU;

struct comp_down_sampled {
    MCU *y_comp;
    MCU *cb_comp;
    MCU *cr_comp;
} COMP_D;


void init_units() 
{
    ;
}

void decode_scan() 
{
    ;
}

void decode_data_unit() 
{
    ;
}

void freadbits() 
{ 
    ;
}

#endif