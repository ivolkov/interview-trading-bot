#include "ipc.h"
#include "conf.h"
#include <stdio.h>
#include <stdlib.h>
#include <mqueue.h>
#include <syslog.h>
#include <errno.h>
#include <string.h>

#define RET_ERR(msg) { syslog(LOG_ERR, "%s: %s", err_prefix, msg); return false; }

/* MQ descriptor */
mqd_t mqd;

/* MQ properties */
struct mq_attr mqstat;

/* MQ buffer */
char *mq_buf;

/* MQ message */
struct mq_struct {
    double market_depth_diff;
    double market_buy_price;
    double market_sell_price;
    time_t tm;
} mq_msg;

bool ipc_init()
{
    const char *err_prefix = "Error @ ipc_init";

    mqd = mq_open(conf_mq_name, O_RDWR | O_CREAT, 0644, NULL);
    if (mqd == (mqd_t)-1) {
        syslog(LOG_ERR, "%s: %s %s", err_prefix, "could not open message queue. Error is: ", strerror(errno));
        return false;
    }

    if (mq_getattr(mqd, &mqstat) == -1) {
        syslog(LOG_ERR, "%s: could not get message queue attributes. Error is: %m", err_prefix);
        return false;
    }

    if ((size_t)mqstat.mq_msgsize < sizeof(mq_msg))
        RET_ERR("message queue buffer is to small")

    if ((mq_buf = malloc(mqstat.mq_msgsize)) == NULL)
        RET_ERR("could not allocate memory for message queue buffer")

    return true;
}

bool ipc_get_prices(double *market_depth_diff, double *market_buy_price, double *market_sell_price, time_t *timestamp)
{
    if (mq_receive(mqd, mq_buf, mqstat.mq_msgsize, NULL) == -1) {
        syslog(LOG_ERR, "Error @ ipc_get_prices: could not receive message from queue. Error is: %m");
        return false;
    }

    memcpy(&mq_msg, mq_buf, sizeof(mq_msg));

    *market_depth_diff = mq_msg.market_depth_diff;
    *market_buy_price = mq_msg.market_buy_price;
    *market_sell_price = mq_msg.market_sell_price;
    *timestamp = mq_msg.tm;

    return true;
}
