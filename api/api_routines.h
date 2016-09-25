#ifndef API_ROUTINES_H_INCLUDED
#define API_ROUTINES_H_INCLUDED

#include <stdbool.h>
#include <stdlib.h>

bool try_fix_error(char *err_string);
bool api_get_responce_public(char *url);
bool api_get_responce_private(char *secret, char *key, char *method);
bool api_parser_get_err_string(char *buf, size_t buf_size);
bool api_parser_compare_err_str(char *sample);

#endif // API_ROUTINES_H_INCLUDED
