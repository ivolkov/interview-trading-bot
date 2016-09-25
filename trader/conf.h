#ifndef CONF_H_INCLUDED
#define CONF_H_INCLUDED

#include <stdbool.h>

#define CONF_MAX_LEN    256

extern char conf_pair[CONF_MAX_LEN];
char conf_curr1[CONF_MAX_LEN];
char conf_curr2[CONF_MAX_LEN];
extern char conf_key[CONF_MAX_LEN];
extern char conf_secret[CONF_MAX_LEN];
extern char conf_mq_name[CONF_MAX_LEN];
extern double conf_trigger;
extern double conf_profit;
extern double conf_fuse;
extern int conf_max_wait;
extern double conf_price_correction;
extern double conf_min_order;

bool conf_read();

#endif // CONF_H_INCLUDED
