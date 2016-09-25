#ifndef TMR_H_INCLUDED
#define TMR_H_INCLUDED

#include <time.h>

void tmr_reset();
time_t tmr_getsec();
void tmr_sleep_1m();

#endif // TMR_H_INCLUDED
