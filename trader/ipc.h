#ifndef IPC_H_INCLUDED
#define IPC_H_INCLUDED

#include <stdbool.h>
#include <time.h>

bool ipc_init();
bool ipc_get_prices(double *market_depth_diff, double *market_buy_price, double *market_sell_price, time_t *timestamp);

#endif // IPC_H_INCLUDED
