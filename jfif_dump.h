#ifndef JFIF_DUMP_H
#define JFIF_DUMP_H

#include "stdlib.h"

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
#define DEBUG_OUT 0

/************
*   TYPES   *
************/

typedef unsigned char B08;
typedef unsigned int  B16;
typedef unsigned int  B32;

typedef struct dhtTable
{
    B08 tableType;
    B08 tableId;
    B08 codeLens[16];
    B16 codeSum;
    B08 *codeSymbols;
} HTABLE; // Huffman table

typedef struct dqtTable
{
    B08 tableId;
    B08 valSize;
    B08 *qVals;
} QTABLE; // Huffman table

typedef struct {
    B08 id;
    B08 sampFreq;
    B08 Hfreq;
    B08 Yfreq;
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

typedef struct node {
    void *data;
    struct node *next;
} Node;

typedef struct list {
    int size;
    Node *head;
} List;

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

List *create_list()
{
    List *new_list = (List*)malloc(sizeof(List));
    new_list->size = 0;
    new_list->head = NULL;
    return new_list;
}

void append(List *list, void *data)
{
    Node *new_head = (Node*)malloc(sizeof(Node));
    new_head->data = data;
    new_head->next = list->head;
    list->head = new_head;
    list->size++;
}

void *pop(List *list)
{
    if (list->size == 0) {
        return NULL;
    }
    Node *pop_node = list->head;
    void *pop_data = pop_node->data;
    list->head = pop_node->next;
    free(pop_node);
    list->size--;
    return pop_data;
}

void *at(List *list, int index)
{
    if (index >= list->size || index < 0) {
        return 0;
    }
    Node *i_node = list->head;
    for (int i = 1; i < (list->size-index); ++i) {
        i_node = i_node->next;
    }
    return i_node->data;
}

void printb(B08 *c, int n, int tabs, int width)
{
    int len = n;
    for (int i = 0; i < (n / width) + 1; ++i) {
        for (int j = 0; j < tabs; ++j) {
            printf("\t");
        }
        for (int k = 0; k < width && len != 0; ++k, --len) {
            printf(" %02x", (B08)c[i*width + k]);
        }
        if (len != 0) printf("\n");
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
    printf("DRI:\n\tLength: %u\n\tInterval: %u\n", DRI.len, DRI.interval);
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
    printf("COM:\n\tText:%s", COM.text);
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
    printf("APP0 (%02x):\n\t", APP0.marker); 
    printf("Length: %d\n\t", APP0.len); 
    printf("ID:"); printb(APP0.id, 5, 0, 8); printf("\n\t");
    printf("Version:"); printb(APP0.version, 2, 0, 8); printf("\n\t"); 
    printf("Units: %d\n\t", APP0.units);
    printf("Xdensity: %d\n\t", APP0.Xdensity);
    printf("Ydensity: %d\n\t", APP0.Ydensity);
    printf("Xthumbnail: %d\n\t", APP0.Xthumbnail);
    printf("Ythumbnail: %d\n\t", APP0.Ythumbnail);
    printf("Thumbnail: ");
    if (APP0.Thumbnail) {
        printf("\n");
        printb(APP0.Thumbnail, thumbnail_len, 1, 8);
    } else {
        printf("None");
    }
    printf("\n");
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
    }

    #if CONSOLE_OUT == 1
    printf("DHT (%02x):\n\t", mr_DHT->marker);
    printf("Length: %d\n", mr_DHT->len);
    for (int k = 0; k < j; ++k) {
        HTABLE table = mr_DHT->tables[k];
        printf("\tTable type: ");
        if (table.tableType == 0) printf("DC\n\t");
        else if (table.tableType == 1) printf("AC\n\t");
        printf("Table id: %d\n\t", table.tableId); 
        printf("Table code lengths:\n"); printb(table.codeLens, 16, 2, 8);
        printf("\n\tTable symbols:\n"); printb(table.codeSymbols, table.codeSum, 2, 16);
        printf("\n");

        if (k+1 != j) printf("\n");
    }
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
    }

    #if CONSOLE_OUT == 1
    printf("DQT (%02x):\n\t", DQT.marker);
    printf("Length: %d\n", DQT.len);
    for (int k = 0; k < j; ++k) {
        QTABLE table = DQT.tables[k];
        printf("\tTable id: %d\n\t", table.tableId);
        printf("Table size: %d\n\t", table.valSize);
        printf("Table values:\n"); printb(table.qVals, table.valSize, 2, 16);
        printf("\n");
        if (k+1 != j) printf("\n");
    }
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
                    comp.Yfreq    = buf[j*3+1+i] & (0x0F);
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
    printf("SOF (%02x):\n\t", SOF0.marker);
    printf("Length: %d\n\t", SOF0.len);
    printf("Precision: %d\n\t", SOF0.precision);
    printf("Height: %d\n\t", SOF0.height);
    printf("Width: %d\n\t", SOF0.width);
    printf("Number of components: %d\n", SOF0.Ncomp);
    for (int j = 0; j < SOF0.Ncomp; ++j) {
        switch (SOF0.compDesc[j].id)
        {
        case 1:
            printf("\tComponent Y:\n\t");
            break;
        
        case 2:
            printf("\tComponent Cb:\n\t");
            break;
        
        case 3:
            printf("\tComponent Cr:\n\t");
            break;
        
        default:
            printf("Unknown component number %d in SOF0\n", SOF0.compDesc[j].id);
            exit(1);
            break;
        }

        printf("   H frequency: %d\n\t", SOF0.compDesc[j].Hfreq);
        printf("   Y frequency: %d\n\t", SOF0.compDesc[j].Yfreq);
        printf("   DQT ID: %d\n", SOF0.compDesc[j].dqtId);
    }
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
    printf("SOS (%02x):\n\t", mr_SOS->marker);
    printf("Length: %d\n\t", mr_SOS->len);
    printf("Number of componenets: %d\n", mr_SOS->Ncomp);
    for (int j = 0; j < mr_SOS->Ncomp; ++j) {
        switch (mr_SOS->compDesc[j].id)
        {
        case 1:
            printf("\tComponent Y:\n\t");
            break;
        
        case 2:
            printf("\tComponent Cb:\n\t");
            break;
        
        case 3:
            printf("\tComponent Cr:\n\t");
            break;
        
        default:
            printf("Unknown component number %d in SOS\n", mr_SOS->compDesc[j].id);
            exit(1);
            break;
        }

        printf("   DC Id: %d\n\t", mr_SOS->compDesc[j].dcId);
        printf("   AC Id: %d\n", mr_SOS->compDesc[j].acId);
    }

    printf("\tSpectral start: %d\n\t", mr_SOS->spectralStart);
    printf("Spectral end: %d\n\t", mr_SOS->spectralEnd);
    printf("Successive approximation: %d\n", mr_SOS->succApprox);
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
            append(DHT_list, mr_DHT);
            init_DHT(fp_jfif, mr_DHT, len);
            break;

        case 0xDA: // SOS
            SOS *mr_SOS = (SOS*)malloc(sizeof(SOS));
            mr_SOS->marker = marker;
            append(SOS_list, mr_SOS);
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

#endif