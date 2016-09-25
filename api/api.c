#include "api.h"
#include "api_routines.h"
#include "api_parser.h"
#include "api_defines.h"
#include <syslog.h>
#include <stdio.h>

#define ERR_LIMIT 20
unsigned int err_cnt;
bool success;

char url[API_SMALL_BUF_LEN];

bool api_get_depth_diff(char *pair, double *diff, double *ask_price, double *bid_price)
{
    sprintf(url, "https://btc-e.com/api/2/%s/ticker", pair);
    return (api_get_responce_public(url) && api_parser_get_depth_diff(diff, ask_price, bid_price));
}

bool api_get_depth_diff_new(char *pair, double *diff, double *ask_price, double *bid_price)
{
    sprintf(url, "https://btc-e.com/api/2/%s/depth", pair);
    return (api_get_responce_public(url) && api_parser_get_depth_diff_new(diff, ask_price, bid_price));
}

bool api_get_trading_fee(char *pair, double *value)
{
    sprintf(url, "https://btc-e.com/api/2/%s/fee", pair);
    return (api_get_responce_public(url) && api_parser_get_trading_gee(value));
}

bool api_get_balance(char *secret, char *key, char *currency1, char *currency2, double *value1, double *value2, long int *open_orders_cnt)
{
    err_cnt = 0;
    while (err_cnt < ERR_LIMIT) {
        if (api_get_responce_private(secret, key, "getInfo") && api_parser_get_info_details(currency1, currency2, value1, value2, open_orders_cnt))
            return true;
        else
            err_cnt++;
    }

    return false;
}

bool api_get_orders(char *secret, char *key, long int *orders, int *orders_num, int orders_limit)
{
    char *method = "ActiveOrders";

    err_cnt = 0;
    while (err_cnt < ERR_LIMIT) {
        if (api_get_responce_private(secret, key, method) && api_parser_get_success(&success)) {
            if (success) {
                if (api_parser_get_active_orders(orders, orders_num, orders_limit))
                    return true;
            } else {
                if (api_parser_compare_err_str("no orders")) {
                    *orders_num = 0;
                    return true;
                }
            }
        }

        err_cnt++;
    }

    return false;
}

bool api_place_order(char *secret, char *key, char *pair, bool buy, double rate, double amount, long int *order_id, double *remains)
{
    char *buy_str = "buy";
    char *sell_str = "sell";
    char *type_str;
    char method[API_SMALL_BUF_LEN];

    if (buy)
        type_str = buy_str;
    else
        type_str = sell_str;

    sprintf(method, "Trade&pair=%s&type=%s&rate=%.3f&amount=%.3f", pair, type_str, rate, amount);

    err_cnt = 0;
    while (err_cnt < ERR_LIMIT) {
        if (api_get_responce_private(secret, key, method))
            if (api_parser_get_success(&success) && success)
                if (api_parser_get_trade_details(order_id, remains))
                    return true;
        err_cnt++;
    }

    return false;
}

bool api_cancel_order(char *secret, char *key, int order_id)
{
    char method[API_SMALL_BUF_LEN];

    sprintf(method, "CancelOrder&order_id=%i", order_id);

    err_cnt = 0;
    while (err_cnt < ERR_LIMIT) {
        if (api_get_responce_private(secret, key, method))
            if (api_parser_get_success(&success))
                if ((success) || (!success && api_parser_compare_err_str("bad status")))
                    return true;

        err_cnt++;
    }

    return false;
}

bool api_cancel_all_orders(char *secret, char *key)
{
    const int max_order_num = 512;
    long int orders[max_order_num];
    int orders_num;

    if (!api_get_orders(secret,key, orders, &orders_num, max_order_num))
        return false;

    int i;
    for (i = 0; i < orders_num; i++)
        if (!api_cancel_order(secret,key, orders[i]))
            return false;

    return true;
}
