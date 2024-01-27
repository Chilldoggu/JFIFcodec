#ifndef JFIF_DUMP_H
#define JFIF_DUMP_H

#include "stdlib.h"
#include "stdio.h"
#include "linked_list.h"

#define APPEND_LIST(OBJ, OBJ_TYPE, MARKER_TYPE, NEW_STRUCT) do { \
            OBJ_TYPE *ptr = OBJ->next;                           \
            while (ptr) {                                        \
                OBJ = ptr;                                       \
                ptr = ptr->next;                                 \
            }                                                    \
            OBJ->next = malloc(sizeof(OBJ_TYPE));                \
            OBJ->obj = NEW_STRUCT;                               \
        } while (0);

#define CONSOLE_OUT 1
#define JFIF_DBUG 1

/*****************
*   PROTOTYPES   *
*****************/

;

/************
*   TYPES   *
************/

typedef unsigned char B08;
typedef unsigned int  B16;
typedef unsigned int  B32;

typedef struct {
    B08 tableType;
    B08 tableId;
    B08 codeLens[16];
    B16 codeSum;
    B08 *codeSymbols;
} HTABLE; // Huffman table

typedef struct dqtTable {
    B08 tableId;
    B08 valSize;
    B08 *qVals;
} QTABLE; // Huffman table

typedef struct {
    B08 id;
    B08 sampFreq;
    B08 Hfreq;
    B08 Vfreq;
    B08 dqtId;
} SOFDESC;

typedef struct {
    B08 id;
    B08 dcId;
    B08 acId;
} SOSDESC;

typedef struct dht {
    B08 marker;
    B16 len;
    B08 table_amount;
    HTABLE *tables;
} DHT;

typedef struct sos {
    B08 marker;
    B16 len;
    B08 Ncomp;
    SOSDESC *compDesc;
    B08 spectralStart;
    B08 spectralEnd;
    B08 succApprox;
    B32 dataLen;
} SOS;

typedef struct generic_marker {
    B08 marker;
    void *body;
} GENERIC_MARKER;

/**************
*   GLOBALS   *
**************/

B32 compressed_data_len = 0;
B08 *compressed_data = 0;
List *SOS_list;
List *DHT_list;

struct soi {
    B08 marker;
} SOI;

struct eoi {
    B08 marker;
} EOI;

struct rst {
    B08 id;
    B08 marker;
} RST[8];

struct dri {
    B08 marker;
    B16 len;
    B16 interval;
} DRI;

struct com {
    B08 marker;
    B16 len;
    B08 *text;
} COM;

struct app0 {
    B08 marker;
    B16 len;
    B08 id[5];
    B08 version[2];
    B08 units;
    B16 Xdensity;
    B16 Ydensity;
    B08 Xthumbnail;
    B08 Ythumbnail;
    B08 *Thumbnail;
} APP0;

struct dqt {
    B08 marker;
    B16 len;
    B08 table_amount;
    QTABLE *tables;
} DQT;

struct sof {
    B08 marker;
    B16 len;
    B08 precision;
    B16 height;
    B16 width;
    B08 Ncomp;
    SOFDESC *compDesc;
} SOF0;

/****************
*   FUNCTIONS   *
****************/

void printb(FILE *stream, B08 *c, int n, int tabs, int width)
{
    int len = n;
    for (int i = 0; i < (n / width) + 1; ++i) {
        for (int j = 0; j < tabs; ++j) {
            fprintf(stream, "\t");
        }
        for (int k = 0; k < width && len != 0; ++k, --len) {
            fprintf(stream, " %02x", (B08)c[i*width + k]);
        }
        if (len != 0) fprintf(stream, "\n");
    }
}

void print_markers(FILE *stream, GENERIC_MARKER *markers, int n)
{
    for (size_t i = 0; i < n; i++) {
        switch (markers[i].marker)
        {
        case 0xE0:
            int thumbnail_len = APP0.len - 16;
            fprintf(stream, "APP0 (%02x):\n\t", APP0.marker); 
            fprintf(stream, "Length: %d\n\t", APP0.len); 
            fprintf(stream, "ID:"); printb(stream, APP0.id, 5, 0, 8); printf("\n\t");
            fprintf(stream, "Version:"); printb(stream, APP0.version, 2, 0, 8); printf("\n\t"); 
            fprintf(stream, "Units: %d\n\t", APP0.units);
            fprintf(stream, "Xdensity: %d\n\t", APP0.Xdensity);
            fprintf(stream, "Ydensity: %d\n\t", APP0.Ydensity);
            fprintf(stream, "Xthumbnail: %d\n\t", APP0.Xthumbnail);
            fprintf(stream, "Ythumbnail: %d\n\t", APP0.Ythumbnail);
            fprintf(stream, "Thumbnail: ");
            if (APP0.Thumbnail) {
                fprintf(stream, "\n");
                printb(stream, APP0.Thumbnail, thumbnail_len, 1, 8);
            } else {
                fprintf(stream, "None");
            }
            fprintf(stream, "\n");
            break;
        
        case 0xDD:
            fprintf(stream, "DRI:\n\tLength: %u\n\tInterval: %u\n", DRI.len, DRI.interval);
            break;

        case 0xC4:
            DHT* DHT_body = (DHT*)markers[i].body;
            fprintf(stream, "DHT (%02x):\n\t", DHT_body->marker);
            fprintf(stream, "Length: %d\n", DHT_body->len);
            for (int k = 0; k < DHT_body->table_amount; ++k) {
                HTABLE table = DHT_body->tables[k];
                fprintf(stream, "\tTable type: ");
                if (table.tableType == 0) fprintf(stream, "DC\n\t");
                else if (table.tableType == 1) fprintf(stream, "AC\n\t");
                fprintf(stream, "Table id: %d\n\t", table.tableId); 
                fprintf(stream, "Table code lengths:\n"); printb(stream, table.codeLens, 16, 2, 8);
                fprintf(stream, "\n\tTable symbols:\n"); printb(stream, table.codeSymbols, table.codeSum, 2, 16);
                fprintf(stream, "\n");

                if (k+1 != DHT_body->table_amount) fprintf(stream, "\n");
            }
            break;
        
        case 0xFE:
            fprintf(stream, "COM:\n\tText:%s", COM.text);
            break;
        
        case 0xDB:
            fprintf(stream, "DQT (%02x):\n\t", DQT.marker);
            fprintf(stream, "Length: %d\n", DQT.len);
            for (int k = 0; k < DQT.table_amount; ++k) {
                QTABLE table = DQT.tables[k];
                fprintf(stream, "\tTable id: %d\n\t", table.tableId);
                fprintf(stream, "Table size: %d\n\t", table.valSize);
                fprintf(stream, "Table values:\n"); printb(stream, table.qVals, table.valSize, 2, 16);
                fprintf(stream, "\n");
                if (k+1 != DQT.table_amount) fprintf(stream, "\n");
            }
            break;

        case 0xC0:
            fprintf(stream, "SOF (%02x):\n\t", SOF0.marker);
            fprintf(stream, "Length: %d\n\t", SOF0.len);
            fprintf(stream, "Precision: %d\n\t", SOF0.precision);
            fprintf(stream, "Height: %d\n\t", SOF0.height);
            fprintf(stream, "Width: %d\n\t", SOF0.width);
            fprintf(stream, "Number of components: %d\n", SOF0.Ncomp);
            for (int j = 0; j < SOF0.Ncomp; ++j) {
                switch (SOF0.compDesc[j].id)
                {
                case 1:
                    fprintf(stream, "\tComponent Y:\n\t");
                    break;
                
                case 2:
                    fprintf(stream, "\tComponent Cb:\n\t");
                    break;
                
                case 3:
                    fprintf(stream, "\tComponent Cr:\n\t");
                    break;
                
                default:
                    fprintf(stream, "Unknown component number %d in SOF0\n", SOF0.compDesc[j].id);
                    exit(1);
                    break;
                }

                fprintf(stream, "   H frequency: %d\n\t", SOF0.compDesc[j].Hfreq);
                fprintf(stream, "   V frequency: %d\n\t", SOF0.compDesc[j].Vfreq);
                fprintf(stream, "   DQT ID: %d\n", SOF0.compDesc[j].dqtId);
            }
            break;

        case 0xDA:
            SOS *SOS_body = (SOS*)markers[i].body;
            fprintf(stream, "SOS (%02x):\n\t", SOS_body->marker);
            fprintf(stream, "Length: %d\n\t", SOS_body->len);
            fprintf(stream, "Number of componenets: %d\n", SOS_body->Ncomp);
            for (int j = 0; j < SOS_body->Ncomp; ++j) {
                switch (SOS_body->compDesc[j].id)
                {
                case 1:
                    fprintf(stream, "\tComponent Y:\n\t");
                    break;
                
                case 2:
                    fprintf(stream, "\tComponent Cb:\n\t");
                    break;
                
                case 3:
                    fprintf(stream, "\tComponent Cr:\n\t");
                    break;
                
                default:
                    fprintf(stream, "Unknown component number %d in SOS\n", SOS_body->compDesc[j].id);
                    exit(1);
                    break;
                }
                fprintf(stream, "   DC Id: %d\n\t", SOS_body->compDesc[j].dcId);
                fprintf(stream, "   AC Id: %d\n", SOS_body->compDesc[j].acId);
            }
            fprintf(stream, "\tSpectral start: %d\n\t", SOS_body->spectralStart);
            fprintf(stream, "Spectral end: %d\n\t", SOS_body->spectralEnd);
            fprintf(stream, "Successive approximation: %d\n", SOS_body->succApprox);
            break;
        
        default:
            fprintf(stream, "Unhandled marker: %02x\n", markers[i].marker);
            break;
        }
    }
}

B32 b_to_int(char *s, int n)
{
    B32 len = 0;
    for (int i = 0; i < n; ++i) {
        len += (B08)s[i] << (8*(n-1-i));
    }
    return len;
}


int init_RST(FILE *fp_jfif, int id)
{
    return 0;
}

int init_DRI(FILE *fp_jfif, B16 len)
{
    char buf[10];
    DRI.len = len; 
    len -= 2;
    fread(buf, len, 1, fp_jfif);
    DRI.interval = b_to_int(buf, 2);

    #if CONSOLE_OUT == 1
    GENERIC_MARKER marker = {DRI.marker, &DRI};
    print_markers(stdout, &marker, 1);
    #endif
    
    return 0;
}

int init_COM(FILE *fp_jfif, B16 len)
{
    COM.len = len; 
    len -= 2;
    COM.text = malloc(len+1);
    fread(COM.text, len, 1, fp_jfif);

    #if CONSOLE_OUT == 1
    GENERIC_MARKER marker = {COM.marker, &COM};
    print_markers(stdout, &marker, 1);
    #endif

    return 0;
}

int init_APP0(FILE *fp_jfif, B16 len)
{
    APP0.len = len;
    len -= 2;
    char buf[len];
    char c;
    int i, segment_len, thumbnail_len;
    i = segment_len = 0;
    fread(buf, len, 1, fp_jfif);

    while (i < len) {
        switch (i) {
            case 0:
                segment_len = 5;
                for (int j = 0; j < segment_len; ++j) {
                    APP0.id[j] = (B08)buf[i+j];
                }
                break;

            case 5:
                segment_len = 2;
                for (int j = 0; j < segment_len; ++j) {
                    APP0.version[j] = (B08)buf[i+j];
                }
                break;
            
            case 7:
                segment_len = 1;
                APP0.units = b_to_int(&buf[i], 1);       
                break;

            case 8:
                segment_len = 4;
                APP0.Xdensity = b_to_int(&buf[i], 2);
                APP0.Ydensity = b_to_int(&buf[i+2], 2);
                break;

            case 12:
                segment_len = 2;
                APP0.Xthumbnail = b_to_int(&buf[i], 1);
                APP0.Ythumbnail = b_to_int(&buf[i+1], 1);
                break;

            default:
                segment_len = len - i;
                thumbnail_len = segment_len;
                APP0.Thumbnail = malloc(segment_len);
                for (int j = 0; j < segment_len; ++j) {
                    APP0.Thumbnail[j] = buf[i+j];
                }
                break;
        }
        i += segment_len;
    }

    #if CONSOLE_OUT == 1
    GENERIC_MARKER marker = {APP0.marker, &APP0};
    print_markers(stdout, &marker, 1);
    #endif

    return 0;
}

int init_DHT(FILE *fp_jfif, DHT *mr_DHT, B16 len)
{
    mr_DHT->len = len;
    len -= 2;
    B08 buf[len];
    int i, j, segment_len, sum;
    i = j = segment_len = sum = 0;
    fread(buf, len, 1, fp_jfif);
    // no idea how to get table amount but I know
    // that they cannot exceed filed size. (oof)
    mr_DHT->tables = malloc(len);

    if (compressed_data_len) compressed_data_len -= mr_DHT->len;

    while (i < len) {
        // Get table type and ID
        segment_len = 1;
        mr_DHT->tables[j].tableType = buf[i] >> 4;
        mr_DHT->tables[j].tableId = buf[i] & 0x0F;
        i += segment_len;

        // Get table code length counts
        segment_len = 16;
        sum = 0;
        for (int k = 0; k < segment_len; ++k, ++i) {
            mr_DHT->tables[j].codeLens[k] = buf[i];
            sum += buf[i];
        }
        mr_DHT->tables[j].codeSum = sum;

        // Get table symbols
        segment_len = sum;
        mr_DHT->tables[j].codeSymbols = malloc(segment_len);
        for (int k = 0; k < segment_len; ++k, ++i) {
            mr_DHT->tables[j].codeSymbols[k] = buf[i];
        }

        ++j;
        mr_DHT->table_amount++;
    }

    #if CONSOLE_OUT == 1
    GENERIC_MARKER marker = {mr_DHT->marker, mr_DHT};
    print_markers(stdout, &marker, 1);
    #endif

    return 0;
}

int init_DQT(FILE *fp_jfif, B16 len)
{
    DQT.len = len;
    len -= 2;
    B08 buf[len];
    int i, j, segment_len;
    i = j = segment_len = 0;
    fread(buf, len, 1, fp_jfif);
    DQT.tables = malloc(len);

    while (i < len) {
        // Get table ID and value sizes
        segment_len = 1;
        DQT.tables[j].tableId = buf[i] & 0x0F;
        DQT.tables[j].valSize = 64 + (buf[i] >> 4) * 64; 
        i += segment_len;

        // Get table code lengths
        segment_len = DQT.tables[j].valSize;
        DQT.tables[j].qVals = malloc(segment_len);
        for (int k = 0; k < segment_len; ++k, ++i) {
            DQT.tables[j].qVals[k] = buf[i];
        }

        ++j;
        DQT.table_amount++;
    }

    #if CONSOLE_OUT == 1
    GENERIC_MARKER marker = {DQT.marker, &DQT};
    print_markers(stdout, &marker, 1);
    #endif

    return 0;
}

int init_SOF0(FILE *fp_jfif, B16 len)
{
    SOF0.len = len;
    len -= 2;
    B08 buf[len];
    int i, segment_len, extra_len;
    i = segment_len = 0;
    fread(buf, len, 1, fp_jfif);

    while (i < len) {
        switch (i) {
            case 0:
                segment_len = 1;
                SOF0.precision = buf[i];
                break;

            case 1:
                segment_len = 4;
                SOF0.height = b_to_int(&buf[i], 2);
                SOF0.width  = b_to_int(&buf[i+2], 2);
                break;
            
            case 5:
                segment_len = 1;
                SOF0.Ncomp = buf[i];
                break;

            case 6:
                segment_len = SOF0.Ncomp * 3;
                SOF0.compDesc = malloc(sizeof(SOFDESC)*SOF0.Ncomp);
                for (int j = 0; j < SOF0.Ncomp; ++j) {
                    // SOFDESC *comp = malloc(sizeof(SOFDESC));
                    SOFDESC comp;
                    comp.id       = buf[j*3+0+i];
                    comp.sampFreq = buf[j*3+0+i];
                    comp.Hfreq    = buf[j*3+1+i] >> 4;
                    comp.Vfreq    = buf[j*3+1+i] & (0x0F);
                    comp.dqtId    = buf[j*3+2+i];
                    SOF0.compDesc[j] = comp;
                }
                break;

            default:
                break;
        }
        i += segment_len;
    }

    #if CONSOLE_OUT == 1
    GENERIC_MARKER marker = {SOF0.marker, &SOF0};
    print_markers(stdout, &marker, 1);
    #endif

    return 0;
}

int init_SOS(FILE *fp_jfif, SOS *mr_SOS, B16 len)
{
    mr_SOS->len = len;
    len -= 2;
    B08 buf[len];
    int i, segment_len;
    i = segment_len = 0;
    fread(buf, len, 1, fp_jfif);

    if (compressed_data) {
        compressed_data_len -= mr_SOS->len;
    } else {
        // Save current position.
        int curr_pos = ftell(fp_jfif);
        // Get to the end of file and get size of the rest of byte stream.
        fseek(fp_jfif, 0, SEEK_END);
        int end_pos = ftell(fp_jfif);
        compressed_data_len = end_pos - curr_pos;
        // Move to the last saved position.
        fseek(fp_jfif, curr_pos, SEEK_SET);
        // Allocate max memory for compressed data stream
        compressed_data = malloc(compressed_data_len);
    }


    mr_SOS->Ncomp = buf[i++];

    segment_len = mr_SOS->Ncomp * 2;
    mr_SOS->compDesc = malloc(sizeof(SOSDESC)*mr_SOS->Ncomp);
    for (int j = 0; j < mr_SOS->Ncomp; ++j) {
        SOSDESC comp; 
        comp.id   = buf[i+j*2];
        comp.dcId = buf[i+1+j*2] >> 4;
        comp.acId = buf[i+1+j*2] & 0x0F;
        mr_SOS->compDesc[j] = comp;
    }
    i += segment_len;

    mr_SOS->spectralStart = buf[i++];
    mr_SOS->spectralEnd = buf[i++];
    mr_SOS->succApprox = buf[i++];

    #if CONSOLE_OUT == 1
    GENERIC_MARKER marker = {mr_SOS->marker, mr_SOS};
    print_markers(stdout, &marker, 1);
    #endif

    return 0;
}

int handle_marker(FILE *fp_jfif, B08 marker, int *data_pos)
{
    // printf("Marker %02x on cur pos: %ld\n", marker, ftell(fp_jfif));

    char str[2];
    int len;

    // Markers without len
    if (marker >= 0xD0 && marker <= 0xD7) {
        int id = marker - 0xD0;
        init_RST(fp_jfif, id);
        return 0;
    }

    switch (marker) {
        case 0x00: // 0xFF for compressed data, don't handle it
            if (compressed_data) { 
                compressed_data[(*data_pos)++] = 0xFFU; // F-FU!
                compressed_data_len++; // compressed_data_len was reduced by 2 before.
            }
            return 0;

        case 0xFF: // Filler
            return 0;

        case 0xD8: // SOI
            SOI.marker = marker;
            return 0;

        case 0xD9: // EOI
            EOI.marker = marker;
            return EOF;
        
        default:
            fread(str, 2, 1, fp_jfif);
            len = b_to_int(str, 2);
            break;
    }

    // markers with len
    switch (marker) {
        case 0xC0: // SOF0 - baseline
            SOF0.marker = marker;
            init_SOF0(fp_jfif, len);
            break;
        
        case 0xC2: // SOF2 - progressive
            SOF0.marker = marker;
            init_SOF0(fp_jfif, len);
            break;

        case 0xC4: // DHT
            DHT *mr_DHT = (DHT*)malloc(sizeof(DHT));
            mr_DHT->marker = marker;
            list_append(DHT_list, mr_DHT);
            init_DHT(fp_jfif, mr_DHT, len);
            break;

        case 0xDA: // SOS
            SOS *mr_SOS = (SOS*)malloc(sizeof(SOS));
            mr_SOS->marker = marker;
            list_append(SOS_list, mr_SOS);
            init_SOS(fp_jfif, mr_SOS, len);
            break;

        case 0xDB: // DQT
            DQT.marker = marker;
            init_DQT(fp_jfif, len);
            break;

        case 0xDD: // DRI
            DRI.marker = marker;
            init_DRI(fp_jfif, len);
            break;
        
        case 0xE0: // APP0
            APP0.marker = marker;
            init_APP0(fp_jfif, len);
            break;

        case 0xFE: // COM
            COM.marker = marker;
            init_COM(fp_jfif, len);
            break;

        default:
            printf("%02x marker undefined.\n", marker);
            return 1;
    }
    return 0;
}

int handle_loop(char* filename)
{
    SOS_list = list_create();
    DHT_list = list_create();

    int r, data_pos;
    unsigned char byte;
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

    FILE *fp_comp = fopen("compressed.txt", "w");
    fwrite(compressed_data, 1, compressed_data_len, fp_comp);

    fclose(fp_jfif);

    return 0;
}

#endif