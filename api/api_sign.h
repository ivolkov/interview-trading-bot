#ifndef API_SIGN_H_INCLUDED
#define API_SIGN_H_INCLUDED

#define MAC_LEN         64
#define MAC_STR_LEN     (MAC_LEN * 2) + 7   // 64 numbers in hex format + "Sign: " prefix + \0 char

void api_get_sign_str(char *secret, char *param_str, char *sign_str);

#endif // API_SIGN_H_INCLUDED
