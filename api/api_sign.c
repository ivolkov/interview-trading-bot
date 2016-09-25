#include "api_sign.h"
#include <stdlib.h>
#include <string.h>
#include <openssl/hmac.h>

void api_get_sign_str(char *secret, char *param_str, char *sign_str)
{
    int i;
    char *prefix = "Sign: ";
    size_t prefix_len = strlen(prefix);
    unsigned char *mac_raw = HMAC(EVP_sha512(), secret, strlen(secret), (unsigned char*)param_str, strlen(param_str), NULL, NULL);

    strcpy(sign_str, prefix);

    for (i = 0; i < MAC_LEN; i++)
        sprintf(&sign_str[(i*2) + prefix_len], "%02x", (unsigned int)mac_raw[i]);

    sign_str[MAC_STR_LEN-1] = '\0';
}
