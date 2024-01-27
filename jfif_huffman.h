#ifndef JFIF_HUFFMAN_H
#define JFIF_HUFFMAN_H

#include <stdio.h>
#include "jfif_dump.h"
#include "huffman.h"

#define min(a, b) (a < b) ? a : b
#define max(a, b) (a > b) ? a : b

typedef struct {
    int Pix_x;
    int Pix_y;
    double **data;
} DATA_UNIT;

typedef struct {
    int H;
    int V;
    int n_units;
    DATA_UNIT *data_units;
} MCU;

struct comp_down_sampled {
    MCU **y_comp;
    MCU **cb_comp;
    MCU **cr_comp;
} COMP_D;

struct bit_reader {
    unsigned char c;
    short int bit;
    FILE *fp;
} B_READER;

int init_huffman(char *filename, struct sof *m_SOF) 
{
    if (m_SOF->Ncomp != 3) {
        return 1;
    }

    /* Initialize bit reader */
    FILE *fp = fopen(filename, "rb");
    B_READER.bit = 0;
    B_READER.c = 0;
    B_READER.fp = fp;

    /* Initialize components */
    SOFDESC YDesc  = m_SOF->compDesc[0];
    SOFDESC CbDesc = m_SOF->compDesc[1];
    SOFDESC CrDesc = m_SOF->compDesc[2];

    int maxH = max(max(YDesc.Hfreq, CbDesc.Hfreq), CrDesc.Hfreq);
    int minH = min(min(YDesc.Hfreq, CbDesc.Hfreq), CrDesc.Hfreq);
    int maxV = max(max(YDesc.Vfreq, CbDesc.Vfreq), CrDesc.Vfreq);
    int minV = min(min(YDesc.Vfreq, CbDesc.Vfreq), CrDesc.Vfreq);

    int MCU_size_x = maxH * 8;
    int MCU_size_y = maxV * 8; 
    int MCU_nx = (m_SOF->width  + MCU_size_x - 1) / MCU_size_x;
    int MCU_ny = (m_SOF->height + MCU_size_y - 1) / MCU_size_y;

    /* Create place holders for data units per each component */
    COMP_D.y_comp  = (MCU**)malloc(sizeof(MCU*) * MCU_ny);
    COMP_D.cb_comp = (MCU**)malloc(sizeof(MCU*) * MCU_ny);
    COMP_D.cr_comp = (MCU**)malloc(sizeof(MCU*) * MCU_ny);
    /* Initialize MCUs */
    for (size_t i = 0; i < MCU_ny; i++) {
        COMP_D.y_comp[i]  = (MCU*)malloc(sizeof(MCU) * MCU_nx);
        COMP_D.cb_comp[i] = (MCU*)malloc(sizeof(MCU) * MCU_nx);
        COMP_D.cr_comp[i] = (MCU*)malloc(sizeof(MCU) * MCU_nx);
        for (size_t j = 0; j < MCU_nx; j++) {
            COMP_D.y_comp[i][j].H  = YDesc.Hfreq;
            COMP_D.y_comp[i][j].V  = YDesc.Vfreq;
            COMP_D.cb_comp[i][j].H = CbDesc.Hfreq;
            COMP_D.cb_comp[i][j].V = CbDesc.Vfreq;
            COMP_D.cr_comp[i][j].H = CrDesc.Hfreq;
            COMP_D.cr_comp[i][j].V = CrDesc.Vfreq;

            COMP_D.y_comp[i][j].n_units  = YDesc.Hfreq  * YDesc.Vfreq;        
            COMP_D.cb_comp[i][j].n_units = CbDesc.Hfreq * CbDesc.Vfreq;        
            COMP_D.cr_comp[i][j].n_units = CrDesc.Hfreq * CrDesc.Vfreq;        

            COMP_D.y_comp[i][j].data_units  = (DATA_UNIT*)malloc(sizeof(DATA_UNIT)*COMP_D.y_comp[i][j].n_units);
            COMP_D.cb_comp[i][j].data_units = (DATA_UNIT*)malloc(sizeof(DATA_UNIT)*COMP_D.cb_comp[i][j].n_units);
            COMP_D.cr_comp[i][j].data_units = (DATA_UNIT*)malloc(sizeof(DATA_UNIT)*COMP_D.cr_comp[i][j].n_units);

            /* Initialize data units for Y component */
            for (size_t k = 0; k < COMP_D.y_comp[i][j].n_units; k++) {
                COMP_D.y_comp[i][j].data_units[k].Pix_x  = maxH/YDesc.Hfreq  * 8;
                COMP_D.y_comp[i][j].data_units[k].Pix_y  = maxV/YDesc.Vfreq  * 8; 
                COMP_D.y_comp[i][j].data_units[k].data   = (double**)malloc(sizeof(double*)*8);
                for (size_t l = 0; l < 8; l++) {
                    COMP_D.y_comp[i][j].data_units[k].data[l] = (double*)malloc(sizeof(double)*8);
                }
            }

            /* Initialize data units for Cb component */
            for (size_t k = 0; k < COMP_D.cb_comp[i][j].n_units; k++) {
                COMP_D.cb_comp[i][j].data_units->Pix_x  = maxH/CbDesc.Hfreq * 8;
                COMP_D.cb_comp[i][j].data_units->Pix_y  = maxV/CbDesc.Vfreq * 8;
                COMP_D.cb_comp[i][j].data_units[k].data = (double**)malloc(sizeof(double*)*8);
                for (size_t l = 0; l < 8; l++) {
                    COMP_D.cb_comp[i][j].data_units[k].data[l] = (double*)malloc(sizeof(double)*8);
                }
            }
            
            /* Initialize data units for Cr component */
            for (size_t k = 0; k < COMP_D.cr_comp[i][j].n_units; k++) {
                COMP_D.cr_comp[i][j].data_units->Pix_x = maxH/CrDesc.Hfreq * 8;
                COMP_D.cr_comp[i][j].data_units->Pix_y = maxV/CrDesc.Vfreq * 8;
                COMP_D.cr_comp[i][j].data_units[k].data   = (double**)malloc(sizeof(double*)*8);
                for (size_t l = 0; l < 8; l++) {
                    COMP_D.cr_comp[i][j].data_units[k].data[l] = (double*)malloc(sizeof(double)*8);
                }
            }
        }
    }

    return 0; 
}

int extend(int magnitude, int additional)
{
    int vt = 1 << (magnitude - 1);
    if (vt > additional) {
        return additional + (-1 << magnitude) + 1;
    }

    return additional;
}

int freadbits(unsigned int *buf, int n) 
{ 
    *buf = 0;
    while (n--) {
        /* read new character from stream if needed */
        if (!B_READER.bit) {
            int ret = fread(&B_READER.c, sizeof(char), 1, B_READER.fp);
            if (!ret) {
                if (feof(B_READER.fp)) {
                    return EOF;
                } else if (ferror(B_READER.fp)) {
                    printf("ERROR OCCURED WHEN READING COMPRESSED DATA STREAM\n");
                    return 1;
                }
            }
        }

        *buf <<= 1;
        *buf += (B_READER.c & (1 << (7-B_READER.bit))) ? 1 : 0;
        B_READER.bit = (B_READER.bit + 1) % 8;
    }

    return 0;
}

void decode_JFIF(char *f_in, HTABLE* h_table)
{
    FILE *fp_in = fopen(f_in, "rb");

    CodeObj *codes[h_table->codeSum];
    for (int i = 0; i < h_table->codeSum; i++) {
        codes[i] = malloc(sizeof(CodeObj));
        codes[i]->c = h_table->codeSymbols[i];
    }

    get_code_lens_from_counts(codes, h_table->codeLens);
    get_code_vals(codes, h_table->codeSum);
}

void decode_scan() 
{
    ;
}

void decode_data_unit() 
{
    ;
}

#endif