#include <stdio.h>
#include <errno.h>
#include <mqueue.h>
#include <time.h>
#include <string.h>
#include <syslog.h>
#include <pthread.h>
#include <libgen.h>
#include <unistd.h>
#include "../api/api.h"
#include "conf.h"

char api_log_ident[128];
const int api_log_facility = LOG_LOCAL4;

void *log_prices(void *arg)
{
    time_t last_log_tm;
    double diff, ask, bid;
    pthread_mutex_t *api_mutex = (pthread_mutex_t *)arg;

    time(&last_log_tm);

    while (1) {
        pthread_mutex_lock(api_mutex);
        if (api_get_depth_diff_new(conf_pair, &diff, &ask, &bid))
            syslog(LOG_INFO | LOG_LOCAL5, "PRICES %s %f %f", conf_pair, bid, ask);
        pthread_mutex_unlock(api_mutex);

        sleep(conf_log_delay);
    }
}

int main(int argc, char *argv[])
{
    int i;
    pthread_t prices_thread;
    pthread_mutex_t api_mutex = PTHREAD_MUTEX_INITIALIZER;

    /* syslog initialization */
    strcpy(api_log_ident, basename(argv[0]));
    openlog(api_log_ident, 0, api_log_facility);

    /* message queue object definition */
    struct mq_struct {
        double market_depth_diff;
        double market_buy_price;
        double market_sell_price;
        time_t tm;
    } mq_item;

    size_t mq_item_len = sizeof(mq_item);

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

    /* read configuration */
    if (!conf_read()) {
        syslog(LOG_ERR, "Error: config file is corrupted");
        return 1;
    }

    mqd_t mqd = mq_open(conf_mq_name, O_RDWR | O_CREAT, 0644, NULL);
    if (mqd == (mqd_t)-1) {
        syslog(LOG_ERR, "Error @ mq_open: %m");
        return 1;
    }

    int res;
    if ((res = pthread_create(&prices_thread, NULL, log_prices, &api_mutex)) != 0) {
        syslog(LOG_ERR, "Error @ pthread_create: %i", res);
        return 1;
    }

    unsigned int err_cntr = 0;

    bool got_data;
    while (1) {
        const unsigned int err_cntr_limit = 100;
        const unsigned int err_timeout_sec = 600;

        /* get market prices */
        pthread_mutex_lock(&api_mutex);
        got_data = (api_get_depth_diff_new(conf_pair, &mq_item.market_depth_diff, &mq_item.market_sell_price, &mq_item.market_buy_price));
        pthread_mutex_unlock(&api_mutex);

        if (got_data) {
            time(&mq_item.tm);

            /* send messages to queue */
            for (i = 0; i < conf_queue_len; i++) {
                if ((mq_send(mqd, (char *)&mq_item, mq_item_len, 0)) == -1)
                    syslog(LOG_ERR, "Error @ mq_send: %m");
            }
        } else {
            err_cntr++;
            syslog(LOG_ERR, "Error counter: %i", err_cntr);
            if (err_cntr > err_cntr_limit) {
                syslog(LOG_INFO, "Errors threshold. Sleeping for 10 minutes.");
                sleep(err_timeout_sec);
                err_cntr = 0;
            }
        }
    }

    return 0;
}

