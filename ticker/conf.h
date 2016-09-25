#ifndef CONF_H_INCLUDED
#define CONF_H_INCLUDED

#include <stdbool.h>

#define CONF_MAX_LEN    256

extern char conf_pair[CONF_MAX_LEN];
extern char conf_mq_name[CONF_MAX_LEN];
extern int conf_queue_len;
extern int conf_log_delay;

bool conf_read();

#endif // CONF_H_INCLUDED
