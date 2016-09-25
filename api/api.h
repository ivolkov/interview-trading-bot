#ifndef API_H_INCLUDED
#define API_H_INCLUDED

#include <stdbool.h>

/* public routines */
bool api_get_depth_diff(char *pair, double *diff, double *ask_price, double *bid_price);
bool api_get_depth_diff_new(char *pair, double *diff, double *ask_price, double *bid_price);
bool api_get_trading_fee(char *pair, double *value);

/* private routines */
bool api_get_balance(char *secret, char *key, char *currency1, char *currency2, double *value1, double *value2, long int *open_orders_cnt);
bool api_get_orders(char *secret, char *key, long int *orders, int *orders_num, int orders_limit);
bool api_place_order(char *secret, char *key, char *pair, bool buy, double rate, double amount, long int *order_id, double *remains);
bool api_cancel_order(char *secret, char *key, int order_id);
bool api_cancel_all_orders(char *secret, char *key);

#endif // API_H_INCLUDED
