#ifndef API_PARSER_ROUTINES_H_INCLUDED
#define API_PARSER_ROUTINES_H_INCLUDED

#include "jsmn/jsmn.h"
#include "api_defines.h"
#include <stdbool.h>

/* parser object */
extern jsmn_parser parser_obj;

/* tokens array */
#define TOKENS_MAX 8192
extern jsmntok_t parser_tokens[TOKENS_MAX];
extern unsigned int parser_tokens_num;

/* raw data buffer */
extern char parser_data_buf[API_LARGE_BUF_LEN];
extern size_t parser_data_buf_size;

/* return token full size (not just direct children count)
 *  token_index -- index of token
 * return -1 if token_index is out of valid range */
int parser_get_token_size(unsigned int token_index);

/* return index of "name" token in "name:value" pair
 *  sample (in) -- string (without closing char) to compare with
 *  parent_index -- index of parent array / object
 * set parent_index to 0 to search in all tokens
 * return -1 if
 *  parent_index is out of valid range
 *  token not found */
int parser_find_name_token(char *sample, unsigned int parent_index);

/* return index of "value" token in "name:value" pair
 *  name (in) -- string (without closing char) value of "name" token in "name:value" pair
 *  type -- type of object to search for
 *  parent_index -- index of parent object
 * set parent_index to 0 to search in all tokens
 * return -1 if
 *  parent_index id out of valid range
 *  token not found */
int parser_find_token(char *name, jsmntype_t type, unsigned int parent_index);

/* convert token to string (with closing char)
 *  token_index -- index of token
 *  buf (out) -- destination buffer
 *  buf_size -- size of destination buffer
 * return false if
 *  token_index is out of valid range
 *  token data size is exceed buf_size */
bool parser_token_to_str(unsigned int token_index, char *buf, size_t buf_size);

/* convert token to int value
 *  token_index -- index of token
 *  val (out) -- converted value
 * return false if
 *  token_index is out of valid range
 *  token data size exceed internal buffer size (STR_BUF_SIZE)
 *  token data can not be converted into long int value */
bool parser_token_to_int(unsigned int token_index, long int *val);

/* convert token to double value
 *  index -- index of token
 *  val (out) -- converted value
 * return false if
 *  token_index is out of valid range
 *  token data size exceed internal buffer size (STR_BUF_SIZE)
 *  token data can not be converted into double value */
bool parser_token_to_double(unsigned int token_index, double *val);

/* compare token to string
 *  str (in) -- string (with close char) to compare with
 *  token_index -- index of token
 * return false if
 *  token_index is out of valid range
 *  given string and token data do not match */
bool parser_comp_token_to_str(char *str, unsigned int token_index);

#endif // API_PARSER_ROUTINES_H_INCLUDED
