#include "exchange.h"
#include "tmr.h"
#include "conf.h"
#include "ipc.h"
#include "../api/api.h"
#include "../api/api_parser.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <syslog.h>
#include <stdlib.h>

#define RET_ERR(msg) { syslog(LOG_ERR, "%s: %s", err_prefix, msg); return false; }
#define EVENT(msg) syslog(LOG_INFO, "%s", msg);
#define TRADING_FEE_MULT (1.0 - (trading_fee / 100.0))

double balance1 = -1.0;
double balance2 = -1.0;
long int open_orders = -1;
unsigned int trades_total = 0;
unsigned int trades_success = 0;
double trading_fee = 0.2;

#define ORDERS_LIMIT 256
long int orders[ORDERS_LIMIT];
int orders_num;

double depth_diff_perc, market_sell_price, market_buy_price;
time_t market_prices_timestamp;

bool update_orders()
{
    const char *err_prefix = "Exchange Error @ update_orders";

    if (!api_get_orders(conf_secret, conf_key, orders, &orders_num, ORDERS_LIMIT))
        RET_ERR("error while updating orders")

    return true;
}

bool update_balance()
{
    const char *err_prefix = "Exchange Error @ update_balance";

    if (!api_get_balance(conf_secret, conf_key, conf_curr1, conf_curr2, &balance1, &balance2, &open_orders))
        RET_ERR("could not updating balance")

    return true;
}

bool update_depth_diff()
{
    const char *err_prefix = "Exchange Error @ update_depth_diff";
    time_t tm, market_prices_timestamp;

    unsigned int cnt = 0;

    do {
        if (!ipc_get_prices(&depth_diff_perc, &market_sell_price, &market_buy_price, &market_prices_timestamp))
            RET_ERR("could not get market prices")

        time(&tm);
        cnt++;
    } while ((tm - market_prices_timestamp) > 0);

    if (cnt != 1)
        syslog(LOG_ERR, "Got prices after %i attempts", cnt);

    return true;
}

bool update_trading_fee()
{
    const char *err_prefix = "Exchange Error @ update_trading_fee";
    double last_trading_fee = trading_fee;

    if (!api_get_trading_fee(conf_pair, &trading_fee))
        RET_ERR("could not get trading fee")

    if (trading_fee != last_trading_fee)
        syslog(LOG_INFO, "Trading fee set to %f", trading_fee);

    return true;
}

/* wait for trading conditions */
bool wait_for_trading_event(bool *price_up)
{
    const char *err_prefix = "Exchange Error @ wait_for_trading_event";

    double last_center_price;
    double current_center_price;

    if (!update_depth_diff())
        RET_ERR("could not update depth diff")

    do {
        last_center_price = CENTER_PRICE;
        if (!update_depth_diff())
            RET_ERR("could not update depth diff")

        if (depth_diff_perc > (trading_fee * 2.0) + conf_fuse) {
            //syslog(LOG_INFO, "Fuse protection: %.3f vs %.3f", depth_diff_perc - (trading_fee * 2.0), conf_fuse);
            depth_diff_perc = 0.0 ;
            sleep(600 + (random() % 100));
        }

    } while (depth_diff_perc < ((trading_fee * 2.0) + conf_trigger));

    current_center_price = CENTER_PRICE;
    *price_up = (last_center_price < current_center_price);

    return true;
}

/* check if order has been filled */
bool check_order(int order_id, bool *completed)
{
    const char *err_prefix = "Exchange Error @ check_order";
    int i;

    if (!update_orders())
        RET_ERR("could not update orders")

    for (i = 0; i < orders_num; i++)
        if (orders[i] == order_id) {
            *completed = false;
            return true;
        }

    *completed = true;
    return true;
}

bool cancel_order(int order_id)
{
    const char *err_prefix = "Exchange Error @ cancel_order";

    if (!api_cancel_order(conf_secret, conf_key, order_id))
        RET_ERR("could not cancel order")

    return true;
}

bool cancel_all_orders()
{
    const char *err_prefix = "Exchange Error @ cancel_all_orders";

    if (!api_cancel_all_orders(conf_secret, conf_key))
        RET_ERR("could not cancel all orders")

    return true;
}

bool instant_order(bool buy, double amount, double *result_price)
{
    const char *err_prefix = "Exchange Error @ instant_order";
    double price;
    double order_remaining;
    long int order_id;
    double balance1_before, balance2_before;
    double delta1, delta2;

    if (buy)
        price = 10000.0;
    else
        price = 1.0;

    if (!update_balance())
        RET_ERR("could not update balance")

    balance1_before = balance1;
    balance2_before = balance2;

    if (!api_place_order(conf_pair, conf_secret, conf_key, buy, price, amount, &order_id, &order_remaining))
        RET_ERR("could not place order")

    if (order_remaining != 0.0)
        RET_ERR("instant order failed")

    if (result_price != NULL) {
        if (!update_balance())
            RET_ERR("could not update balance")

        if ((delta1 = fabs(balance1 - balance1_before)) == 0.0)
            RET_ERR("invalid delta1 value")

        if ((delta2 = fabs(balance2 - balance2_before)) == 0.0)
            RET_ERR("invalid delta2 value")

        *result_price = (delta2 / delta1);
    }

    return true;
}

bool init()
{
    const char *err_prefix = "Exchange Error @ init";

    srand(time(NULL) | getpid());

    if (!conf_read())
        RET_ERR("could not read configuration file")

    if (!update_trading_fee())
        RET_ERR("could not update trading fee")

    if (!ipc_init())
        RET_ERR("could not initialize IPC system")

    if (!update_depth_diff())
        RET_ERR("could not update market prices")

    if (!update_orders())
        RET_ERR("could not update active orders")

    if (orders_num != 0) {
        EVENT("Found orders at startup")
        if (!wait_for_all_orders())
            RET_ERR("could not wait for active orders to complete")
    }

    if (!update_balance())
        RET_ERR("could not update balance")

    return true;
}

bool place_orders(double *round_profit)
{
    const char *err_prefix = "Exchange Error @ place_orders";

    long int bid_order_id;
    bool bid_order_completed;
    long int ask_order_id;
    bool ask_order_completed;
    bool price_up;
    bool skip_bid, skip_ask;

    double bid_price, ask_price;
    double bid_volume, ask_volume;
    double center_price, modifier, correction;
    double order_volume_limit1, order_volume_limit2;

    double bid_order_remaining;
    double ask_order_remaining;

    char event_descr[128];

    *round_profit = 0.0;

    if (!update_trading_fee())
        RET_ERR("could not update trading fee")

    if (!wait_for_trading_event(&price_up))
        RET_ERR("could not wait for trading event to occur")

    /* determine ask/bid prices */
    center_price = CENTER_PRICE;

    modifier = 1.0 + ( (trading_fee + (conf_profit / 2.0)) / 100.0 );
    bid_price = center_price / modifier;
    ask_price = center_price * modifier;

    correction = (bid_price - market_sell_price) * conf_price_correction / 100.0;
    if (!price_up)
        correction = -correction;

    bid_price += correction;
    ask_price += correction;

    /* determine order volume limits */
    order_volume_limit1 = TOTAL_BALANCE1 / 2.0;
    order_volume_limit2 = TOTAL_BALANCE2 / 2.0;

    /* calculate orders volume */
    ask_volume = balance1 * 0.98;
    if (ask_volume > order_volume_limit1)
        ask_volume = order_volume_limit1 * 0.98;

    bid_volume = (balance2 / bid_price) * 0.98;
    if (bid_volume > order_volume_limit2)
        bid_volume = order_volume_limit2 * 0.98;

    /* check for exchange limits */
    skip_bid = (bid_volume < conf_min_order);
    skip_ask = (ask_volume < conf_min_order);

    /* place orders */
    if (!skip_bid)
        if (!api_place_order(conf_secret, conf_key, conf_pair, true, bid_price, bid_volume, &bid_order_id, &bid_order_remaining))
            RET_ERR("could not place bid order")

    if (!skip_ask)
        if (!api_place_order(conf_secret, conf_key, conf_pair, false, ask_price, ask_volume, &ask_order_id, &ask_order_remaining))
            RET_ERR("could not place ask order")

    bid_order_completed = (bid_order_remaining == 0.0);
    ask_order_completed = (ask_order_remaining == 0.0);

    /* add history record */
    if (price_up)
        strcpy(event_descr, "Price got up");
    else
        strcpy(event_descr, "Price drop down");

    syslog(LOG_INFO, "%s (%.3f : %.3f, delta %.3f). Orders %.5f @ %.3f : %.5f @ %.3f", event_descr, market_buy_price, market_sell_price, depth_diff_perc - (trading_fee * 2.0), bid_volume, bid_price, ask_volume, ask_price);

    if (skip_bid)
        EVENT("Skipping bid order")
    if (skip_ask)
        EVENT("Skipping ask order")

    /* wait for orders to complete */
    if (!wait_for_all_orders())
        RET_ERR("could not finish all orders")

    /* cancel incomplete orders */
    if (!skip_ask) {
        if (!check_order(ask_order_id, &ask_order_completed))
            RET_ERR("could not check status of ask order")

        if (!ask_order_completed) {
            if (cancel_order(ask_order_id))
                EVENT("ask order canceled")
            else
                RET_ERR("could not cancel ask order")
        }
    }

    if (!skip_bid) {
        if (!check_order(bid_order_id, &bid_order_completed))
            RET_ERR("could not check status of bid order")

        if (!bid_order_completed) {
            if (cancel_order(bid_order_id))
                EVENT("bid order canceled")
            else
                RET_ERR("could not cancel bid order")
        }
    }

    /* if any order completed */

    if ( (!skip_bid && !skip_ask) && (bid_order_completed || ask_order_completed) ) {
        trades_total++;

        if (bid_order_completed && ask_order_completed) {
            trades_success++;
        } else if (!bid_order_completed) {
            if (!instant_order(true, bid_volume, &bid_price))
                RET_ERR("could not place instant bid order")
        } else {
            if (!instant_order(false, ask_volume, &ask_price))
                RET_ERR("could not place instant ask order")
        }

        if (!update_balance())
            RET_ERR("could not update balance")
    }

    /* delay 5 m */
    tmr_reset();
    while (tmr_getsec() < 300) {
        sleep(5);
    }

    /* bid result */
    double diff_usd = -(bid_volume * bid_price);
    double diff_btc = bid_volume * TRADING_FEE_MULT;

    /* ask result */
    diff_usd += (ask_volume * ask_price * TRADING_FEE_MULT);
    diff_btc -= ask_volume;

    *round_profit = diff_usd + (diff_btc * ask_price * TRADING_FEE_MULT);

    return true;
}

bool wait_for_all_orders()
{
    const char *err_prefix = "Exchange Error @ wait_for_all_orders";

    tmr_reset();

    do {
        if (!update_orders())
            RET_ERR("could not update orders")

        if (orders_num == 0)
            return true;

        sleep(5);
    } while (tmr_getsec() < conf_max_wait);

    RET_ERR("timeout")
}
