#include "tmr.h"
#include <unistd.h>

time_t tstart;

void tmr_reset()
{
    time(&tstart);
}

time_t tmr_getsec()
{
    time_t tnow;
    time(&tnow);

    return tnow - tstart;
}

void tmr_sleep_1m()
{
    tmr_reset();
    while (tmr_getsec() < 60)
        sleep(1) ;
}
