#include "conf.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <syslog.h>
#include <libconfig.h>

#define CONF_FNAME      "config"

#define CONF_PAIR_LEN       7

#define CONF_MIN_QUEUE_LEN  1
#define CONF_MAX_QUEUE_LEN  100

#define CONF_MIN_LOG_DELAY  1
#define CONF_MAX_LOG_DELAY  86400

#define INIT_ERR(msg) { syslog(LOG_ERR, "%s: %s. %s", err_prefix, msg, config_error_text(&config)); res = false; }

char conf_pair[CONF_MAX_LEN];
char conf_mq_name[CONF_MAX_LEN];
int conf_queue_len = 10;
int conf_log_delay = 60;

bool conf_read()
{
    const char *err_prefix = "Error @ conf_read";
    bool res = true;

    config_t config;
    const char *str_value;

    config_init(&config);

    if (!config_read_file(&config, CONF_FNAME))
        INIT_ERR("could not read configuration file")

    if (res) {
        if (config_lookup_string(&config, "pair", &str_value)) {
            if (strlen(str_value) == CONF_PAIR_LEN) {
                strcpy(conf_pair, str_value);
                sprintf(conf_mq_name, "/%s_price_events", conf_pair);
            } else
                INIT_ERR("'pair' value has wrong length")
        } else
            INIT_ERR("could not read 'pair' valie")

        if (config_lookup_int(&config, "queue_len", &conf_queue_len)) {
            if ((conf_queue_len < CONF_MIN_QUEUE_LEN) || (conf_queue_len > CONF_MAX_QUEUE_LEN))
                INIT_ERR("'queue_len' value is out of range")
        } else
            INIT_ERR("could not read 'queue_len' value")

        if (config_lookup_int(&config, "log_delay" ,&conf_log_delay)) {
            if ((conf_log_delay < CONF_MIN_LOG_DELAY) || (conf_log_delay > CONF_MAX_LOG_DELAY))
                INIT_ERR("'log_delay' value is out of range")
        } else
            INIT_ERR("could not read 'log_delay' value")
    }

    config_destroy(&config);
    return res;
}
