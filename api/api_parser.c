#include "api_parser.h"
#include "api_parser_routines.h"
#include <string.h>
#include <syslog.h>

#define RET_ERR(msg) { syslog(LOG_ERR, "%s: %s", err_prefix, msg); return false; }
#define RET_DUMP_ERR(msg) { api_parser_log_last_response(); RET_ERR(msg); }

void api_parser_init()
{
    unsigned int i;

    jsmn_init(&parser_obj);
    parser_tokens_num = 0;
    parser_data_buf[0] = '\0';
    parser_data_buf_size = 0;

    for (i = 0; i < TOKENS_MAX; i++)
        parser_tokens[i].start = -1;
}

bool api_parser_append(char *data, size_t len)
{
    const char *err_prefix = "API Error @ api_parser_append";
    unsigned int i;

    //printf("parsing data: %s\n", data);

    /* one byte at the end of parser_data_buf is reserved for closing char */
    if (len > (API_LARGE_BUF_LEN - parser_data_buf_size - 2))
        RET_ERR("not enough memory were provided to process data")

    memcpy(&parser_data_buf[parser_data_buf_size], data, len);
    parser_data_buf_size += len;
    jsmnerr_t res = jsmn_parse(&parser_obj, parser_data_buf, parser_data_buf_size, parser_tokens, TOKENS_MAX);


    switch (res) {
    /* Not enough tokens were provided */
    case JSMN_ERROR_NOMEM:
        parser_tokens_num = 0;
        RET_ERR("not enough tokens were provided to process data")

    /* Invalid character inside JSON string */
    case JSMN_ERROR_INVAL:
        parser_tokens_num = 0;
        RET_DUMP_ERR("invalid character detected inside JSON string")

    /* The string is not a full JSON packet, more bytes expected */
    case JSMN_ERROR_PART:
        return true;

    /* packet was parsed succesfully */
    default:
        parser_data_buf[parser_data_buf_size] = '\0';
        parser_data_buf_size++;

        for (i = 0; i < TOKENS_MAX; i++)
            if (parser_tokens[i].start == -1)
                break;
            else
                parser_tokens_num++;

        return true;
    }
}

bool api_parser_done()
{
    return (parser_tokens_num);
}

bool api_parser_get_err_string(char *buf, size_t buf_size)
{
    const char *err_prefix = "API Error @ api_parser_get_err_string";
    int err_str_index;

    if ((err_str_index = parser_find_token("error", JSMN_STRING, 0)) == -1)
        RET_DUMP_ERR("could not find 'error' token")

    if (!parser_token_to_str(err_str_index, buf, buf_size))
        RET_DUMP_ERR("could not convert token 'error' value to string")

    return true;
}

bool api_parser_compare_err_str(char *sample)
{
    char str_buf[API_SMALL_BUF_LEN];
    return ((api_parser_get_err_string(str_buf, API_SMALL_BUF_LEN)) && (strncmp(str_buf, sample, strlen(sample)) == 0));
}

void api_parser_log_last_response()
{
    syslog(LOG_DEBUG, "API Debug: Server last response is: \n%s", parser_data_buf);
}

bool api_parser_get_depth_diff(double *diff_perc, double *ask_price, double *bid_price)
{
    const char *err_prefix = "API Error @ api_parser_get_depth_diff";
    int ticker_index;
    int buy_index;
    int sell_index;

    if ((ticker_index = parser_find_token("ticker", JSMN_OBJECT, 0)) == -1)
        RET_DUMP_ERR("could not find 'ticker' token")

    if ((buy_index = parser_find_token("buy", JSMN_PRIMITIVE, ticker_index)) == -1)
        RET_DUMP_ERR("could not find 'buy' token")

    if ((sell_index = parser_find_token("sell", JSMN_PRIMITIVE, ticker_index)) == -1)
        RET_DUMP_ERR("could not find 'sell' token")

    if (!parser_token_to_double(buy_index, ask_price))
        RET_DUMP_ERR("could not convert token 'buy' value to double")

    if (!parser_token_to_double(sell_index, bid_price))
        RET_DUMP_ERR("could not convert token 'sell' value to double")

    if (*bid_price == 0)
        RET_DUMP_ERR("token 'sell' value can not be 0")

    *diff_perc = ((*ask_price * 100.0) / *bid_price) - 100.0;
    return true;
}

bool api_parser_get_depth_diff_new(double *diff_perc, double *ask_price, double *bid_price)
{
    const char *err_prefix = "API Error @ api_parser_get_depth_diff_new";
    int asks_index = parser_find_token("asks", JSMN_ARRAY, 0);
    int bids_index = parser_find_token("bids", JSMN_ARRAY, 0);

    if (asks_index == -1)
        RET_DUMP_ERR("token 'asks' not found")

    if (parser_tokens[asks_index].size == 0)
        RET_DUMP_ERR("size of 'asks' token can not be 0")

    if ((parser_tokens[asks_index + 1].type != JSMN_ARRAY) || (parser_tokens[asks_index + 1].size != 2))
        RET_DUMP_ERR("could not find 'asks' token values array")

    if (!parser_token_to_double(asks_index + 2, ask_price))
        RET_DUMP_ERR("could not convert first item from 'asks' token values array to double")

    if (*ask_price == 0)
        RET_DUMP_ERR("token 'buy' value can not be 0")

    if (bids_index == -1)
        RET_DUMP_ERR("could not find 'bids' token")

    if (parser_tokens[bids_index].size == 0)
        RET_DUMP_ERR("size of 'bids' token can not be 0")

    if ((parser_tokens[bids_index + 1].type != JSMN_ARRAY) || (parser_tokens[bids_index + 1].size != 2))
        RET_DUMP_ERR("could not find 'bids' token values array")

    if (!parser_token_to_double(bids_index + 2, bid_price))
        RET_DUMP_ERR("could not convert first item from 'bids' token values array to double")

    if (*bid_price == 0)
        RET_DUMP_ERR("token 'sell' value can not be 0")

    if (diff_perc != NULL)
        *diff_perc = ((*ask_price * 100.0) / *bid_price) - 100.0;

    return true;
}

bool api_parser_get_trading_gee(double *value)
{
    const char *err_prefix = "API Error @ api_get_trading_fee";
    int fee_prim_index;

    if ((fee_prim_index = parser_find_token("trade", JSMN_PRIMITIVE, 0)) == -1)
        RET_DUMP_ERR("could not find 'trade' token")

    if (!parser_token_to_double(fee_prim_index, value))
        RET_DUMP_ERR("could not convert 'trade' token value to double")

    return true;
}

bool api_parser_get_success(bool *value)
{
    const char *err_prefix = "API Error @ api_parser_get_success";
    long int success;
    int success_prim_index;

    if ((success_prim_index = parser_find_token("success", JSMN_PRIMITIVE, 0)) == -1)
        RET_DUMP_ERR("could not find 'success' token")

    if (!parser_token_to_int(success_prim_index, &success))
        RET_DUMP_ERR("could not convert 'success' token value to int")

    *value = (success == 1);
    return true;
}

bool api_parser_get_info_details(char *currency1, char *currency2, double *value1, double *value2, long int *open_orders_cnt)
{
    const char *err_prefix = "API Error @ api_parser_get_info_details";
    bool got_value1 = false;
    bool got_value2 = false;
    int return_obj_index;
    int open_orders_prim_index;
    int i;

    /* find "return" token */
    if ((return_obj_index = parser_find_token("return", JSMN_OBJECT, 0)) == -1)
        RET_DUMP_ERR("could not find 'return' token")

    /* find "open_orders" token */
    if ((open_orders_prim_index = parser_find_token("open_orders", JSMN_PRIMITIVE, return_obj_index)) == -1)
        RET_DUMP_ERR("could not find 'open_orders' token")

    /* parse "open_orders" token value */
    if (!parser_token_to_int(open_orders_prim_index, open_orders_cnt))
        RET_DUMP_ERR("could not convert 'open_orders' token value to int")

    /* note: do not add +1 to second parameter -- the last one can not be useful */
    for (i = return_obj_index + 1; i < return_obj_index + parser_tokens[return_obj_index].size; i++)
        if ((parser_tokens[i].type == JSMN_STRING) && (parser_tokens[i+1].type == JSMN_PRIMITIVE)) {
            if (!got_value1 && parser_comp_token_to_str(currency1, i)) {
                if (parser_token_to_double(i+1, value1)) {
                    got_value1 = true;
                } else {
                    syslog(LOG_ERR, "%s: could not convert %s token value to double", err_prefix, currency1);
                    api_parser_log_last_response();
                    return false;
                }

            } else if (!got_value2 && parser_comp_token_to_str(currency2, i)) {
                if (parser_token_to_double(i+1, value2)) {
                    got_value2 = true;
                } else {
                    syslog(LOG_ERR, "%s: could not convert %s tokan value to double", err_prefix, currency2);
                    api_parser_log_last_response();
                    return false;
                }
            }

            if (got_value1 && got_value2)
                return true;
        }

    return false;
}

bool api_parser_get_active_orders(long *orders, int *orders_num, int orders_limit)
{
    const char *err_prefix = "API Error @ api_parser_get_active_orders";
    int return_obj_index;
    int i;
    unsigned int ptr;

    if ((return_obj_index = parser_find_token("return", JSMN_OBJECT, 0)) == -1)
        RET_DUMP_ERR("could not find 'return' token")

    *orders_num = (parser_tokens[return_obj_index].size / 2);

    if ((parser_tokens[return_obj_index].size / 2) > orders_limit)
        RET_DUMP_ERR("number of orders exceed given orders limit")

    ptr = return_obj_index + 1;

    for (i = 0; i < *orders_num; i++) {
        if ((ptr >= parser_tokens_num) || (parser_tokens[ptr].type != JSMN_STRING) || (parser_tokens[ptr+1].type != JSMN_OBJECT))
            RET_DUMP_ERR("invalid token type")

        if (!parser_token_to_int(ptr, &orders[i]))
            RET_DUMP_ERR("could not convert 'order number' token value to int")

        ptr += parser_get_token_size(ptr+1) + 1;
    }

    return true;
}

bool api_parser_get_trade_details(long int *order_id, double *remains)
{
    const char *err_prefix = "API Error @ api_parser_get_trade_details";
    int return_obj_index;
    int remains_prim_index;
    int order_id_prim_index;

    if ((return_obj_index = parser_find_token("return", JSMN_OBJECT, 0)) == -1)
        RET_DUMP_ERR("could not find 'return' token")

    if ((remains_prim_index = parser_find_token("remains", JSMN_PRIMITIVE, return_obj_index)) == -1)
        RET_DUMP_ERR("could not find 'remains' token")

    if (!parser_token_to_double(remains_prim_index, remains))
        RET_DUMP_ERR("could not convert 'remains' token value to double")

    if ((order_id_prim_index = parser_find_token("order_id", JSMN_PRIMITIVE, return_obj_index)) == -1)
        RET_DUMP_ERR("could not find 'order_id' token")

    if (!parser_token_to_int(order_id_prim_index, order_id))
        RET_DUMP_ERR("could not convert 'order_id' token value to int")

    return true;
}

bool api_parser_get_funds(char *currency1, char *currency2, double *value1, double *value2)
{
    const char *err_prefix = "API Error @ parser_get_funds";
    int return_obj_index;
    int funds_obj_index;
    int value_prim_index;

    if ((return_obj_index = parser_find_token("return", JSMN_OBJECT, 0)) == -1)
        RET_DUMP_ERR("could not find 'return' token")

    if ((funds_obj_index = parser_find_token("funds", JSMN_OBJECT, return_obj_index)) == -1)
        RET_DUMP_ERR("could not find 'funds' token")

    if ((value_prim_index = parser_find_token(currency1, JSMN_PRIMITIVE, funds_obj_index)) == -1) {
        syslog(LOG_ERR, "%s: could not find %s token", err_prefix, currency1);
        api_parser_log_last_response();
        return false;
    }

    if (!parser_token_to_double(value_prim_index, value1)) {
        syslog(LOG_ERR, "%s: could not convert %s token value to double", err_prefix, currency1);
        api_parser_log_last_response();
        return false;
    }

    if ((value_prim_index = parser_find_token(currency2, JSMN_PRIMITIVE, funds_obj_index)) == -1) {
        syslog(LOG_ERR, "%s: could not find %s token", err_prefix, currency2);
        api_parser_log_last_response();
        return false;
    }

    if (!parser_token_to_double(value_prim_index, value2)) {
        syslog(LOG_ERR, "%s: could not convert %s token value to double", err_prefix, currency2);
        api_parser_log_last_response();
        return false;
    }

    return true;
}
