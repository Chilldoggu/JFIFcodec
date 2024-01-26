#ifndef HUFFMAN_H
#define HUFFMAN_H

#include <stdio.h>
#include <stdlib.h>
#include "linked_list.h"

#define HUFFMAN_DEBUG 1
#define MAX_CODE_LEN 16 // JPEG DHT field constraint

/*
*   Process of encoding with Huffman algorithm:
*
*      1. Get character appearance frequencies from input. Function: `get_char_freq()`
*
*      2. Use those frequencies to generate code lengths (aka. depth in huffman tree) 
*         for each input element. Function: `get_code_lens_from_input()` with help of
*         `merge_two_lowest_frequencies()`.
*    
*      3. Having array of elements with length<->character relation we need to sort it
*         ascendingly by code_length (from most frequent to least frequent character).
*
*      4. Having that array we can begin to generate codes for corresponding
*         input elements. Function: `get_code_vals()`.
*    
*      5. Array of lengths has a lot of redundancy so we can shorten it to an array of
*         frequencies of each code (code counts), where max code length is 16 (JFIF
*         specification). Function: `get_code_counts()`.
*            > Each element in code count array has `code_length = index + 1` relation.
*    
*      6. By using array of code counts it's trivial to convert it to code lengths and
*         then to code values that can be used to map each input element into corresponding
*         character from character table. Functions: `get_code_lens_from_counts()`. 
*            > Sorted by code length character table of characters from input stream has to be
*              saved somewhere in order for the decoding process to be successful.
*        
*   Process of decoding (JFIF DHT marker):
*
*      1. Firstly create an array of CodeObj with a length equal to number of symbols from
*         character table.
*            > Those characters from charactr table are already sorted so don't worry about it.
*            > Remember to assign each symbol from tble to (CodeObj*)x->c.
*
*      2. Create code lengths and then code values for each CodeObj in array using 
*         `get_code_lens_from_counts()` and `get_code_vals()`.
*   
*      3. Congratulaions now you have a structure that you can use to decode compressed 
*         data stream, just map each element using `decode()` function.
*/

/************
*   TYPES   *
************/

typedef struct codeobj {
    unsigned char c;
    unsigned char length; // Length == depth in tree 
    unsigned short val;
} CodeObj;

typedef struct freqobj {
    List *codes;
    int freq;
} FreqObj;

/*****************
*   PROTOTYPES   *
*****************/

/* Interface */
int encode(char *f_ouput, char *input, int input_size);
int compress_data(FILE* fp_dst, char *input, CodeObj *code_arr[], int code_arr_size);
char* decode(char *f_in);
char* decompress_data(FILE *fp_in, char char_table[], CodeObj *codes[], char code_counts[], int table_size);
void print_code_val(unsigned int val, int len);

/* Static functions */
static int list_isinside(List *l1, char c);
static char find_dummy(char *input);
static char *append_dummy(char *input, int input_size, char dummy);
static List *get_code_lens_from_input(char *input);
static int sort_code_arr_by_len(CodeObj *code_arr[], const int size, char dummy);
int get_code_vals(CodeObj *code_arr[], const int size);
static unsigned char *get_code_counts(CodeObj *code_arr[], const int size);
int get_code_lens_from_counts(CodeObj *code_arr[], char *code_counts);

/****************
*   FUNCTIONS   *
*****************/

int encode(char *f_ouput, char *input, int input_size)
{
    char dummy = find_dummy(input);
    input = append_dummy(input, input_size, dummy);

    /* I'm not returning an array because i want to save structure's size */
    List *codes = get_code_lens_from_input(input);
    /* Covert list to array */
    int code_arr_size = codes->size;
    CodeObj *code_arr[code_arr_size];
    for (int i = 0; i < codes->size; ++i) {
        code_arr[i] = list_at(codes, i);
    }
    list_free(codes);

    sort_code_arr_by_len(code_arr, code_arr_size, dummy);
    get_code_vals(code_arr, code_arr_size);
    unsigned char *code_counts = get_code_counts(code_arr, code_arr_size);

    FILE *fp_dst = fopen(f_ouput, "wb");

    /* Save counts (size == 16 byte) and char table (size == sum of counts) */
    fwrite(code_counts, sizeof(char), MAX_CODE_LEN, fp_dst);

    /* Save sorted char table */
    for (int i = 0; i < code_arr_size; ++i) {
        fputc(code_arr[i]->c, fp_dst);
    }

    /* Compress data */
    compress_data(fp_dst, input, code_arr, code_arr_size);

    #if HUFFMAN_DEBUG == 1
    char debug_c;
    int sum = 0;
    printf("\n");
    for (int i = 0; (debug_c = input[i]) != '\0'; ++i) {
        for (int j = 0; j < code_arr_size; ++j) {
            CodeObj *c_obj = code_arr[j];
            if (c_obj->c == debug_c) sum += c_obj->length;
        }
    }
    printf("\ncode_arr table:\n");
    for (int j = 0; j < code_arr_size; ++j) {
        printf("Char \"%c\" with code_len %d has a code ", code_arr[j]->c, code_arr[j]->length);
        print_code_val(code_arr[j]->val, code_arr[j]->length);
        printf("\n");
    }
    printf("Sum: %d\n", sum);
    #endif


    free(code_counts);
    free(input);
    fclose(fp_dst);

    return 0;
}

char* decode(char *f_in)
{
    FILE *fp_in = fopen(f_in, "rb");

    /* Read code counts */
    char code_counts[MAX_CODE_LEN];
    fread(code_counts, 1, MAX_CODE_LEN, fp_in);

    /* Read and save char table */
    int table_size = 0;
    for (int i = 0; i < MAX_CODE_LEN; i++) {
        table_size += code_counts[i];
    } 
    char char_table[table_size];
    fread(char_table, 1, table_size, fp_in);
    CodeObj *codes[table_size];
    for (int i = 0; i < table_size; i++) {
        codes[i] = malloc(sizeof(CodeObj));
        codes[i]->c = char_table[i];
    }

    /* Get code values */
    get_code_lens_from_counts(codes, code_counts);
    get_code_vals(codes, table_size);

    /* Decode */
    char *decompressed_data = decompress_data(fp_in, char_table, codes, code_counts, table_size);

    /* Clear */
    fclose(fp_in);
    for (int i = 0; i < table_size; i++) {
        free(codes[i]);
    }

    return decompressed_data;
}

int compress_data(FILE* fp_dst, char *input, CodeObj *code_arr[], int code_arr_size)
{
    /* Compressed data */
    /* 1. Find segment length in bits and how many byets are needed to save. */
    char c;
    int segment_len, padding;
    segment_len = padding = 0;
    for (int i = 0; (c = input[i]) != '\0'; ++i) {
        for (int j = 0; j < code_arr_size; ++j) {
            if (c == code_arr[j]->c) {
                segment_len += code_arr[j]->length;
                break;
            }
        } 
    }

    int n_bytes = segment_len / 8;
    if (segment_len % 8) n_bytes++;
    padding = n_bytes*8 - segment_len;

    char buffer[n_bytes];
    for (int i = 0; i < n_bytes; ++i) buffer[i] = 0;
    
    /* 2. Compress data stream */
    int acc = 0;
    for (int i = 0; (c = input[i]) != '\0'; ++i) {
        short code_val;
        short code_len;

        /* find character that matches character from input */
        for (int j = 0; j < code_arr_size; ++j) {
            if (c == code_arr[j]->c) {
                code_val = code_arr[j]->val;
                code_len = code_arr[j]->length;
                break;
            }
        }

        // Fit 16 bit codes inside 8 bit buffer elements continuously
        for (int j = code_len; j > 0; --j) {
            if (code_val & (1 << (j - 1))) {
                buffer[acc/8] |= (1 << (7 - (acc%8)));
            }
            acc++;
        }
    }

    /* fill spare bits fil 1s */
    for (int i = 0; i < padding; i++)
    {
        buffer[acc/8] |= (1 << (7 - (acc%8)));
        acc++;
    }

    /* 3. Save everything to a file */
    fwrite(buffer, sizeof(char), n_bytes, fp_dst);
    
    #if HUFFMAN_DEBUG == 1
    printf("===== ENCODING =====\n\n");
    printf("Segment len in bits: %d, n_bytes: %d\nCompressed data stream: ", segment_len, n_bytes);
    for (int i = 0; i < n_bytes; ++i) {
        print_code_val(buffer[i], 8);
        printf(" ");
    }
    #endif

    return 0;
}

char* decompress_data(FILE *fp_in, char char_table[], CodeObj *codes[], char code_counts[], int table_size)
{
    /* 1. Create decode helper object */
    int max_len = MAX_CODE_LEN;
    for (int i = MAX_CODE_LEN; i > 0; i--) {
        if (code_counts[i-1]) {
            max_len = i;
            break;
        }
    }
    
    typedef struct decodeobj {
        int len;
        unsigned short int min_code;
        unsigned short int max_code;
        int first_val_index;
    } DecodeObj;

    DecodeObj DecodeObj_arr[max_len];
    
    for (int i = 0; i < max_len; i++) {
        DecodeObj_arr[i].len = i+1;
        if (code_counts[i]) {
            for (int j = 0; j < table_size; j++) {
                if (codes[j]->length == i+1) {
                    DecodeObj_arr[i].first_val_index = j;
                    DecodeObj_arr[i].min_code        = codes[j]->val;
                    DecodeObj_arr[i].max_code        = codes[j+code_counts[i]-1]->val;
                    break;
                }
            }
            
        } else {
            DecodeObj_arr[i].first_val_index = -1;
            DecodeObj_arr[i].min_code        = -1;
            DecodeObj_arr[i].max_code        = -1;
        }
    }
    
    /* 2. Read compressed data and decode*/
    unsigned short int decode_val = 0;
    char *decompressed_data = malloc(0xFFFFF); // Assigning a buffor of 1MB just to be sure I read everything
    char not_finished = 1;
    int data_size = 0;
    int decode_len = 0;
    while (not_finished) {
        unsigned char c = fgetc(fp_in);
        for (int i = 7; i >= 0; i--) {
            decode_val |= (c >> i) & 1;
            DecodeObj check_decode = DecodeObj_arr[decode_len];
            if (check_decode.first_val_index != -1 && \
                check_decode.min_code <= decode_val && \
                check_decode.max_code >= decode_val) {
                if (decode_val == codes[table_size-1]->val) {
                    not_finished = 0;
                    break;
                }
                int diff = decode_val - check_decode.min_code; 
                decompressed_data[data_size++] = char_table[check_decode.first_val_index + diff];
                decode_len = 0;
                decode_val = 0;
            } else {
                decode_val <<= 1;
                decode_len++;
            }
        }
    }

    #if HUFFMAN_DEBUG == 1
    printf("\n===== DECODING =====\n\n");
    for (int i = 0; i < table_size; i++)
    {
        printf("Char: %c, code len: %d, code: ", codes[i]->c, codes[i]->length);
        print_code_val(codes[i]->val, codes[i]->length);
        printf("\n");
    }
    printf("\nStructure of helper object DecodeObj:\n");
    for (int i = 0; i < max_len; i++)
    {
        printf("Len: %d, first_index: %d, min_val: ", DecodeObj_arr[i].len, DecodeObj_arr[i].first_val_index);
        (DecodeObj_arr[i].first_val_index == -1) ? printf("-1, ") : print_code_val(DecodeObj_arr[i].min_code, DecodeObj_arr[i].len);
        printf(", max_val: ");
        (DecodeObj_arr[i].first_val_index == -1) ? printf("max_val: -1") : print_code_val(DecodeObj_arr[i].max_code, DecodeObj_arr[i].len);
        printf("\n");
    }
    printf("\nDecompressed data: ");
    for (int i = 0; i < data_size; i++)
    {
        printf("%c", decompressed_data[i]);
    }
    printf("\n");
    #endif

    return decompressed_data;
}

void print_code_val(unsigned int val, int len)
{
    int shift = 1;
    while (len) {
        if (val & (shift << (len-1))) {
            printf("1");
        } else {
            printf("0");
        }
        len--;
    }
}

int list_isinside(List *l1, char c)
{
    for (int i = 0; i < l1->size; ++i) {
        char *c_l1 = (char*)list_at(l1, i);
        if (*c_l1 == c) {
            return 0; // in l1f
        }
    }
    return 1; // not in buff
}

static List *get_char_freq(char *input)
{
    char c;
    List *char_dupes = list_create(); // Remembre to free
    List *c_freqs = list_create(); // Each element consists of len 1 Codes list and a frequency value

    for (int i = 0; (c = input[i]) != '\0'; ++i) {
        if(!list_isinside(char_dupes, c)) { // char in dupe buffer
            // Increase character frequency
            for (int j = 0; j < c_freqs->size; ++j) {
                FreqObj *i_freq = (FreqObj*)list_at(c_freqs, j);
                char *freq_char = (char*)list_at(i_freq->codes, 0);
                if (*freq_char == c) {
                    i_freq->freq++;
                }
            }
        } else { // char not in dupe buffer
            // Append c_freqs by a new character
            char *data = (char*)malloc(sizeof(char));
            *data = c;
            list_append(char_dupes, data);
            CodeObj *code = (CodeObj*)malloc(sizeof(CodeObj));
            code->c = c;
            code->length = 0;
            code->val = 0xFFFF; // Give code wrong value so it can be overwritten later
            FreqObj *freq = (FreqObj*)malloc(sizeof(FreqObj));
            freq->freq = 1; 
            freq->codes = (List*)list_create();
            list_append(freq->codes, code);
            list_append(c_freqs, freq);
        }
    }

    // Free dupe buffer
    list_full_free(char_dupes);

    return c_freqs;
}

static FreqObj *find_lowest(List *freqs)
{
    FreqObj *min = NULL;
    for (int i = freqs->size-1; i >= 0; --i) {
        FreqObj *new = list_at(freqs, i);
        if (min == NULL || min->freq > new->freq) {
            min = new;
        }
    }

    return min;
}


static int merge_two_lowest_frequencies(List *freqs)
{
    FreqObj *lowest = find_lowest(freqs);
    int f1 = lowest->freq;
    List *cat_codes = lowest->codes;
    if (list_remove_obj(freqs, lowest)) {
        printf("Couldn't remove obj");
        exit(-1);
    }
    FreqObj *new_lowest = find_lowest(freqs);
    list_cat(new_lowest->codes, cat_codes);
    new_lowest->freq += f1;

    List *new_codes = new_lowest->codes;
    for (int i = 0; i < new_codes->size; ++i) {
        CodeObj *app_code = (CodeObj*)list_at(new_codes, i);
        app_code->length++;
    }

    return 0;
}

static List *get_code_lens_from_input(char *input)
{
    List *freqs = get_char_freq(input);

    while (freqs->size != 1) {
        merge_two_lowest_frequencies(freqs);
    }

    return ((FreqObj*)list_at(freqs, 0))->codes;
}

static int sort_code_arr_by_len(CodeObj *code_arr[], const int size, char dummy)
{
    /* Selection sort */
    for (int i = 0; i < size; ++i) {
        CodeObj *min_obj = NULL;
        int min_index = 0;

        /* check for lowest val with index */
        for (int j = size - 1; j >= i; --j) {
            unsigned char min_val = code_arr[j]->length;
            if (min_obj == NULL || min_obj->length > min_val) {
                min_obj = code_arr[j];
                min_index = j;
            }
        }

        /* swap */
        CodeObj *tmp = code_arr[i];
        code_arr[i] = min_obj;
        code_arr[min_index] = tmp;
    }

    /* 
    *   Ensure that dummy value is on the end of the array.
    */
    if (code_arr[size-1]->c != dummy) {
        for (int i = 0; i < size; ++i) {
            if (code_arr[i]->c == dummy) {
                CodeObj *tmp = code_arr[i];
                code_arr[i] = code_arr[size-1];
                code_arr[size-1] = tmp;
            }
        }
    }

    return 0;
}

int get_code_vals(CodeObj *code_arr[], const int size)
{
    short huffman_code = 0;
    unsigned char code_len_counter = 1;

    for (int i = 0; i < size; ++i) {
        if (code_arr[i]->length == code_len_counter) {
            code_arr[i]->val = huffman_code;
            huffman_code++;
        } else {
            huffman_code = huffman_code << 1;
            code_len_counter++;
            --i; // works so who cares
        }
    }

    return 0;
}

// Useful only for encoding not decoding
static unsigned char *get_code_counts(CodeObj *code_arr[], const int size)
{
    unsigned char *counts = (unsigned char*)malloc(sizeof(char)*MAX_CODE_LEN);
    for (int i = 0; i < MAX_CODE_LEN; ++i) {
        counts[i] = 0;
    }

    for (int i = 0; i < size; ++i) {
        counts[code_arr[i]->length - 1]++;
    }

    return counts;
}

int get_code_lens_from_counts(CodeObj *code_arr[], char *code_counts)
{
    int index = 0;
    for (int i = 0; i < MAX_CODE_LEN; ++i) {
        unsigned char length = i + 1;
        for (int j = 0; j < code_counts[i]; ++j) {
            code_arr[index++]->length = length;
        }
    }

    return 0;
}

/* Find smallest value not used in the string */
static char find_dummy(char *input)
{
    char c;
    char i = 1;
    for (; i < 256; ++i) {
        char not_found = 1;
        for (int j = 0; (c = input[j]) != '\0'; ++j) {
            if (c == (char)i) {
                not_found = 0;
                break;
            }
        }

        if (not_found) break;
    }

    return (char)i;
}

static char *append_dummy(char *input, int input_size, char dummy)
{
    int i = 0;
    char *dst = (char*)malloc(input_size+1);
    while (input[i] != '\0') {
        dst[i] = input[i]; 
        i++;
    }
    dst[i] = dummy;
    dst[i+1] = '\0';

    return dst;
}

#endif