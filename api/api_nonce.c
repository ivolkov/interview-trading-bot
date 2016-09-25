#include "api_nonce.h"
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>

#define NONCE_FNAME "nonce.conf"

long nonce = -1;

void save_nonce()
{
    FILE *f = fopen(NONCE_FNAME, "wb");
    if (f == NULL) {
        syslog(LOG_ERR, "API Error @ save_none: could not open file %s", NONCE_FNAME);
        return;
    }

    fseek(f, 0, SEEK_SET);
    fwrite(&nonce, sizeof(nonce), 1, f);
    fflush(f);
    fclose(f);
}

long api_get_nonce()
{
    FILE *f;

    if (nonce == -1) {
        if ((((f = fopen(NONCE_FNAME, "rb")) != NULL)) && (fread(&nonce, sizeof(nonce), 1, f) == 1))
            fclose(f);
        else
            nonce = 1;
    }

    nonce++;
    save_nonce();
    return nonce;
}

void api_set_nonce(long value)
{
    nonce = value;
    save_nonce();
}
