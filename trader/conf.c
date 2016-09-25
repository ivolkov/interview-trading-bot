#include "conf.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <syslog.h>
#include <libconfig.h>

#define CONF_FNAME      "config"

#define CONF_PAIR_LEN   7
#define CONF_CURR_LEN   3
#define CONF_KEY_LEN    44
#define CONF_SECRET_LEN 64

#define CONF_TRIGGER_MIN    0.001
#define CONF_TRIGGER_MAX    5.0

#define CONF_PROFIT_MIN     0.002
#define CONF_PROFIT_MAX     5.001

#define CONF_FUSE_MIN       0.003
#define CONF_FUSE_MAX       5.002

#define CONF_MAX_WAIT_MIN    0
#define CONF_MAX_WAIT_MAX    2592000     // one month

#define CONF_PRICE_CORRECTION_MIN   -500.0
#define CONF_PRICE_CORRECTION_MAX   500.0

#define CONF_MIN_ORDER_MIN  0.000000001
#define CONF_MIN_ORDER_MAX  100000000.0

#define INIT_ERR(msg) { syslog(LOG_ERR, "%s: %s. %s", err_prefix, msg, config_error_text(&config)); res = false; }

char conf_pair[CONF_MAX_LEN];
char conf_curr1[CONF_MAX_LEN];
char conf_curr2[CONF_MAX_LEN];
char conf_key[CONF_MAX_LEN];
char conf_secret[CONF_MAX_LEN];
char conf_mq_name[CONF_MAX_LEN];
double conf_trigger = CONF_TRIGGER_MAX;
double conf_profit = CONF_PROFIT_MIN;
double conf_fuse = CONF_FUSE_MIN;
int conf_max_wait = CONF_MAX_WAIT_MAX;
double conf_price_correction = 0.0;
double conf_min_order = CONF_MIN_ORDER_MIN;

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
        /* pair */
        if (!config_lookup_string(&config, "pair", &str_value))         INIT_ERR("could not read 'pair' configuration parameter")
        if (strlen(str_value) != CONF_PAIR_LEN)                         INIT_ERR("configuration parameter 'pair' has wrong length")

        strcpy(conf_pair, str_value);
        sprintf(conf_mq_name, "/%s_price_events", conf_pair);

        strncpy(conf_curr1, conf_pair, CONF_CURR_LEN);
        conf_curr1[CONF_CURR_LEN] = '\0';

        strncpy(conf_curr2, conf_pair + 4, CONF_CURR_LEN);
        conf_curr2[CONF_CURR_LEN] = '\0';

        /* key */
        if (!config_lookup_string(&config, "key", &str_value))
            INIT_ERR("could not read 'key' configuration parameter")
        if (strlen(str_value) != CONF_KEY_LEN)
            INIT_ERR("configuration parameter 'key' has wrong length")

        strcpy(conf_key, str_value);

        /* secret */
        if (!config_lookup_string(&config, "secret", &str_value))
            INIT_ERR("could not read 'secret' configuration parameter")
        if (strlen(str_value) != CONF_SECRET_LEN)
            INIT_ERR("configuration parameter 'secret' has wrong length")

        strcpy(conf_secret, str_value);

        /* trigger */
        if (!config_lookup_float(&config, "trigger", &conf_trigger))
            INIT_ERR("could not read 'trigger' configuration parameter")
        if ((conf_trigger < CONF_TRIGGER_MIN) || (conf_trigger > CONF_TRIGGER_MAX))
            INIT_ERR("configuration parameter 'trigger' is out of valid range")

        /* profit */
        if (!config_lookup_float(&config, "profit", &conf_profit))
            INIT_ERR("could not read 'profit' configuration parameter")
        if ((conf_profit < CONF_PROFIT_MIN) || (conf_profit > CONF_PROFIT_MAX))
            INIT_ERR("configuration parameter 'trigger' is out of valid range")

        /* fuse */
        if (!config_lookup_float(&config, "fuse", &conf_fuse))
            INIT_ERR("could not read 'fuse' configuration parameter")
        if ((conf_fuse < CONF_FUSE_MIN) || (conf_fuse > CONF_FUSE_MAX))
            INIT_ERR("configuration parameter 'fuse' is out of valid range")
        if (conf_fuse <= conf_trigger)
            INIT_ERR("configuration parameter 'fuse' value should be less than 'trigger' value")

        /* max_wait */
        if (!config_lookup_int(&config, "max_wait", &conf_max_wait))
            INIT_ERR("could not read 'max_wait' configuration parameter")

        if ((conf_max_wait < CONF_MAX_WAIT_MIN) || (conf_max_wait > CONF_MAX_WAIT_MAX))
            INIT_ERR("configuration parameter 'wax_wait' is out of valid range")

        /* price_correction */
        if (!config_lookup_float(&config, "price_correction", &conf_price_correction))
            INIT_ERR("could not read 'price_correction' configuration parameter")

        if ((conf_price_correction < CONF_PRICE_CORRECTION_MIN) && (conf_price_correction > CONF_PRICE_CORRECTION_MAX))
            INIT_ERR("configuration parameter 'price_correction' is out of valid range")

        /* min_order */
        if (!config_lookup_float(&config, "min_order", &conf_min_order))
            INIT_ERR("could not read 'min_order' configuration parameter")
        if ((conf_min_order < CONF_MIN_ORDER_MIN) && (conf_min_order > CONF_MIN_ORDER_MAX))
            INIT_ERR("configuration parameter 'min_order' is out of valid range")
    }

    config_destroy(&config);
    return res;
}
