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

typedef struct codeobj {
    unsigned char c;
    unsigned char length; // Length == depth in tree 
    short val;
} CodeObj;

typedef struct freqobj {
    List *codes;
    int freq;
} FreqObj;

int _in_list_str(List *buf, char c)
{
    for (int i = 0; i < buf->size; ++i) {
        char *c_buf = (char*)list_at(buf, i);
        if (*c_buf == c) {
            return 0; // in buff
        }
    }
    return 1; // not in buff
}

List *get_char_freq(char *input)
{
    char c;
    List *char_dupes = list_create(); // Remembre to free
    List *c_freqs = list_create(); // Each element consists of len 1 Codes list and a frequency value

    for (int i = 0; (c = input[i]) != '\0'; ++i) {
        if(!_in_list_str(char_dupes, c)) { // char in dupe buffer
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
    for (int i = 0; i < char_dupes->size; ++i) {
        free(list_at(char_dupes, i));
    }
    list_free(char_dupes);

    return c_freqs;
}

FreqObj *_find_lowest(List *freqs)
{
    FreqObj *min = NULL;
    for (int i = 0; i < freqs->size; ++i) {
        FreqObj *new = list_at(freqs, i);
        if (min == NULL || min->freq > new->freq) {
            min = new;
        }
    }

    return min;
}

int _remove_list_obj(List *freqs, FreqObj* obj)
{
    Node *list_iter = freqs->head; 
    if ((FreqObj*)list_iter->data == obj) {
        freqs->head = list_iter->next; 
        free(list_iter->data);
        free(list_iter);
        freqs->size--;
        return 0;
    }

    for (int i = 0; i < freqs->size - 1; ++i) {
        if ((FreqObj*)list_iter->next->data == obj) {
            Node *tmp = list_iter->next;
            list_iter->next = list_iter->next->next;
            free(tmp->data);
            free(tmp);
            freqs->size--;
            return 0;
        }
        list_iter = list_iter->next;
    }
    return 1;
}

int _cat_lists(List *l1, List *l2)
{
    for (int i = 0; i < l2->size; ++i) {
        CodeObj *code = list_at(l2, i);
        list_append(l1, code);
    }
    list_free(l2);

    return 0;
}

int _merge_two_lowest_frequencies(List *freqs)
{
    FreqObj *lowest = _find_lowest(freqs);
    int f1 = lowest->freq;
    List *cat_codes = lowest->codes;
    if (_remove_list_obj(freqs, lowest)) {
        printf("Couldn't remove obj");
        exit(-1);
    }
    FreqObj *new_lowest = _find_lowest(freqs);
    _cat_lists(new_lowest->codes, cat_codes);
    new_lowest->freq += f1;

    List *new_codes = new_lowest->codes;
    for (int i = 0; i < new_codes->size; ++i) {
        CodeObj *app_code = (CodeObj*)list_at(new_codes, i);
        app_code->length++;
    }

    return 0;
}

List *get_code_lens_from_input(char *input)
{
    List *freqs = get_char_freq(input);

    while (freqs->size != 1) {
        _merge_two_lowest_frequencies(freqs);
    }

    return ((FreqObj*)list_at(freqs, 0))->codes;
}

int sort_code_arr_by_len(CodeObj *code_arr[], const int size, char dummy)
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
    *   Ensure that dummy value is on the end of the array so we can discard it later,
    *   because standard doesn't allow codes with only ones. 
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

void _print_code_val(unsigned int val, int len)
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
unsigned char *get_code_counts(CodeObj *code_arr[], const int size)
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

// Maybe change it so that we save lengths to code_arr
int get_code_lens_from_counts(CodeObj *code_arr[], int *code_counts)
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

char find_dummy(char *input)
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

char *append_dummy(char *input, int input_size, char dummy)
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

int encode(char *f_ouput, char *input, int input_size)
{
    char dummy = find_dummy(input);
    // char dummy = '.';
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
    /* get rid of last value that's the last element (we know it from sort array) */
    free(code_arr[--code_arr_size]);
    /* bring back original input */
    input[input_size-1] = '\0';
    unsigned char *code_counts = get_code_counts(code_arr, code_arr_size);

    FILE *fp_dst = fopen(f_ouput, "wb");

    /* Save counts (size = 16 byte) and char table (size = sum of counts) */
    fwrite(code_counts, sizeof(char), MAX_CODE_LEN, fp_dst);

    /* Save sorted char table */
    for (int i = 0; i < code_arr_size; ++i) {
        fputc(code_arr[i]->c, fp_dst);
    }

    /* Compressed data */
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

    /* save everything to a file */
    fwrite(buffer, sizeof(char), n_bytes, fp_dst);
    

    printf("Segment_len: %d, n_bytes: %d\n", segment_len, n_bytes);
    for (int i = 0; i < n_bytes; ++i) {
        _print_code_val(buffer[i], 8);
        printf(" ");
    }

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
    for (int j = 0; j < code_arr_size; ++j) {
        printf("Char \"%c\" with code_len %d has a code ", code_arr[j]->c, code_arr[j]->length);
        _print_code_val(code_arr[j]->val, code_arr[j]->length);
        printf("\n");
    }
    printf("Sum: %d\n", sum);
    #endif


    free(code_counts);
    free(input);
    fclose(fp_dst);

    return 0;
}

int main()
{
    char *input = "A MAN A PLAN A CANAL PANAMA";
    char *f_out = "encode.txt";
    int input_size = 0;

    while (input[input_size++] != '\0');

    encode(f_out, input, input_size);

    return 0;
}