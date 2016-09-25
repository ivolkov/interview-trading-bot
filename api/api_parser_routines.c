#include "api_parser_routines.h"
#include <errno.h>
#include <string.h>
#include <stdlib.h>

jsmn_parser parser_obj;

jsmntok_t parser_tokens[TOKENS_MAX];
unsigned int parser_tokens_num;

char parser_data_buf[API_LARGE_BUF_LEN];
size_t parser_data_buf_size = 0;

char str_buf[API_SMALL_BUF_LEN];

#define TOKEN_LEN(index) (parser_tokens[index].end - parser_tokens[index].start)

int parser_get_token_size(unsigned int token_index)
{
    unsigned int ptr = token_index + 1;
    int token_size;
    int i;

    if (ptr >= parser_tokens_num)
        return -1;

    for (i = 0; i < parser_tokens[token_index].size; i++) {
        switch (parser_tokens[ptr].type) {
        case JSMN_OBJECT:
        case JSMN_ARRAY:
            if ((token_size = parser_get_token_size(ptr)) == -1)
                return -1;
            ptr += token_size;
            break;
        default:
            ptr++;
            break;
        }
    }

    return (ptr - token_index);
}

int parser_find_name_token(char *sample, unsigned int parent_index)
{
    unsigned int ptr = parent_index + 1;
    int token_size;
    int i;

    if (parent_index >= parser_tokens_num)
        return -1;

    for (i = 0; i < parser_tokens[parent_index].size; i++) {
        switch (parser_tokens[ptr].type) {
        case JSMN_OBJECT:
        case JSMN_ARRAY:
            if ((token_size = parser_get_token_size(ptr)) == -1)
                return -1;
            ptr += token_size;
            break;
        default:
            if ((parser_tokens[ptr].type == JSMN_STRING) && (strncmp(sample, &parser_data_buf[parser_tokens[ptr].start], TOKEN_LEN(ptr)) == 0))
                return ptr;
            ptr++;
        }
    }

    return -1;
}

int parser_find_token(char *name, jsmntype_t type, unsigned int parent_index)
{
    int str_index = parser_find_name_token(name, parent_index);
    unsigned int index = str_index + 1;

    if ((str_index == -1) || (index >= parser_tokens_num) || (parser_tokens[index].type != type))
        return -1;
    else
        return index;
}

bool parser_token_to_str(unsigned int token_index, char *buf, size_t buf_size)
{
    if (token_index > parser_tokens_num)
        return false;

    if ((size_t)(TOKEN_LEN(token_index) + 1) >= buf_size)
        return false;

    strncpy(buf, parser_data_buf + parser_tokens[token_index].start, TOKEN_LEN(token_index));
    buf[TOKEN_LEN(token_index)] = '\0';

    return true;
}

bool parser_token_to_int(unsigned int token_index, long int *val)
{
    if (token_index >= parser_tokens_num)
        return false;

    if (TOKEN_LEN(token_index) >= API_SMALL_BUF_LEN)
        return false;

    if (!parser_token_to_str(token_index, str_buf, API_SMALL_BUF_LEN))
        return false;

    *val = strtol(str_buf, NULL, 10);
    if (errno == ERANGE)
        return false;

    return true;
}

bool parser_token_to_double(unsigned int token_index, double *val)
{
    if (token_index >= parser_tokens_num)
        return false;

    if (TOKEN_LEN(token_index) >= API_SMALL_BUF_LEN)
        return false;

    if (!parser_token_to_str(token_index, str_buf, API_SMALL_BUF_LEN))
        return false;

    *val = strtod(str_buf, NULL);
    if (errno == ERANGE)
        return false;

    return true;
}

bool parser_comp_token_to_str(char *str, unsigned int token_index)
{
    if (token_index >= parser_tokens_num)
        return false;

    return (strncmp(str, parser_data_buf + parser_tokens[token_index].start, TOKEN_LEN(token_index)) == 0) ;
}
