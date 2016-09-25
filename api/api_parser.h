#ifndef API_PARSER_H_INCLUDED
#define API_PARSER_H_INCLUDED

#include <stdbool.h>
#include <stddef.h>

/* parser initialization
 * this function must be called before any other parser functions */
void api_parser_init();

/* append data to parser buffer
 *  data (in) -- another block of data
 *  len -- size of data
 * return false if
 *  not enough tokens were provided to fill data
 *  invalid character meet inside JSON string */
bool api_parser_append(char *data, size_t len);

/* return true if given data were parsed successfully */
bool api_parser_done();

/* extract error message from JSON data
 * return false if
 *  token 'error' not found
 *  token value contains invalid data */
bool api_parser_get_err_string(char *buf, size_t buf_size);

bool api_parser_compare_err_str(char *sample);

/* copy last server response to api_last_server_response */
void api_parser_log_last_response();

/* application-specific public routines */
bool api_parser_get_depth_diff(double *diff_perc, double *ask_price, double *bid_price);
bool api_parser_get_depth_diff_new(double *diff_perc, double *ask_price, double *bid_price);
bool api_parser_get_trading_gee(double *value);

/* application-specific private routines */
bool api_parser_get_success(bool *value);
bool api_parser_get_info_details(char *currency1, char *currency2, double *value1, double *value2, long int *open_orders_cnt);
bool api_parser_get_active_orders(long *orders, int *orders_num, int orders_limit);
bool api_parser_get_trade_details(long int *order_id, double *remains);
bool api_parser_get_funds(char *currency1, char *currency2, double *value1, double *value2);

#endif // API_PARSER_H_INCLUDED
