#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <syslog.h>
#include <string.h>
#include <libgen.h>
#include <sys/types.h>
#include <unistd.h>
#include "../api/api.h"
#include "exchange.h"
#include "conf.h"

#define LOG_MSG(msg) syslog(LOG_ERR, "%s: %s", err_prefix, msg)
#define ERR_CONT(msg) { LOG_MSG(msg); continue; }
#define ERR_EXIT(msg) { LOG_MSG(msg); return 1; }

char api_log_ident[128];
const int api_log_facility = LOG_LOCAL4;
bool cancel_orders = false;

int main(int argc, char *argv[])
{
    const char *err_prefix = "Trader Error @ main";
    double start_balance1, start_balance2;
    double prev_balance2;
    double trade_profit, round_profit;

    strcpy(api_log_ident, basename(argv[0]));
    openlog(api_log_ident, LOG_CONS | LOG_PID | LOG_NDELAY, api_log_facility);

    /* create PID file */
    pid_t pid = getpid();
    FILE *pid_file = fopen("pid", "w");

    if (pid_file == NULL) {
        syslog(LOG_ERR, "Error: could not open file pid");
        return 1;
    }

    if (fprintf(pid_file, "%i", pid) < 0) {
        syslog(LOG_ERR, "Error: could not write to pid file");
        return 1;
    }

    fclose(pid_file);

    if (!init())
        ERR_EXIT("initialization failed, exiting")

    if (!update_depth_diff())
        ERR_EXIT("could not update market prices, exiting")

    start_balance1 = balance1;
    start_balance2 = balance2;
    trade_profit = 0.0;

    syslog(LOG_INFO, "starting with %.5f %s, %.5f %s", balance1, conf_curr1, balance2, conf_curr2);
    syslog(LOG_NOTICE, "%.3f", TOTAL_BALANCE2);

    while (1) {
        /* re-read configuration file */
        if (!conf_read())
            ERR_EXIT("could not update configuration, exiting")

        if (cancel_orders) {
            if (!cancel_all_orders())
                ERR_CONT("could not cancel active orders")

            cancel_orders = false;
        }

        if (!update_depth_diff())
            ERR_CONT("could not update market prices")

        if (!update_balance())
            ERR_CONT("could not update balance")

        prev_balance2 = TOTAL_BALANCE2;

        if (!place_orders(&round_profit)) {
            cancel_all_orders();
            syslog(LOG_INFO, "%s: could not place orders, reinitializing", err_prefix);
            init();
            continue;
        }

        if (!update_balance())
            ERR_EXIT("could not update balance, exiting")

        if (!update_depth_diff())
            ERR_EXIT("could not update market prices, exiting")

        trade_profit += round_profit;

        syslog(LOG_INFO, "%i trades. %s: %.5f (delta %.5f). %s: %.5f (delta %.5f). Total value delta: %.3f. Round: %.3f", trades_success, conf_curr1, balance1, (balance1 - start_balance1), conf_curr2, balance2, (balance2 - start_balance2), TOTAL_BALANCE2 - prev_balance2, round_profit);
        syslog(LOG_NOTICE, "%.5f %.5f", TOTAL_BALANCE2, round_profit);
    }
}
