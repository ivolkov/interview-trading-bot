#ifndef EXCHANGE_H_INCLUDED
#define EXCHANGE_H_INCLUDED

#include <stdbool.h>

extern double balance1;
extern double balance2;
extern long int open_orders;

extern double market_sell_price;
extern double market_buy_price;

unsigned int trades_total;
unsigned int trades_success;

#define CENTER_PRICE (market_sell_price + ((market_buy_price - market_sell_price) / 2.0))

#define TOTAL_BALANCE1 (balance1 + (balance2 / CENTER_PRICE))
#define TOTAL_BALANCE2 (balance2 + (balance1 * CENTER_PRICE))

#define FEE_MULT (1.0 - (trading_fee / 100.0))

/* if any function return false, trading should stop */
bool init();
bool update_orders();
bool update_balance();
bool update_depth_diff();
bool place_orders(double *round_profit);
bool cancel_all_orders();
bool wait_for_all_orders();

#endif // EXCHANGE_H_INCLUDED
