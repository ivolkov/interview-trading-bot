#include "api_routines.h"
#include "api_parser.h"
#include "api_defines.h"
#include "api_sign.h"
#include "api_nonce.h"
#include <string.h>
#include <curl/curl.h>
#include <syslog.h>
#include <errno.h>

#define RET_ERR(msg) { syslog(LOG_ERR, "%s: %s", err_prefix, msg); return false; }

size_t curl_callback(void *buffer, size_t size, size_t nmemb, void *userp)
{
    size_t len = size * nmemb;

    if (api_parser_append(buffer, len))
        return len;
    else
        return 0;
}

bool try_fix_error(char *err_string)
{
    const char *err_prefix = "API Parser Error @ try_fix_error";
    char sbuf[API_SMALL_BUF_LEN];
    int i, j;

    /* sample string for "invalid nonce parameter:
     * "invalid nonce parameter; on key:23, you send:23" */
    char *sample_nonce_err = "invalid nonce parameter; on key:";
    if (api_parser_compare_err_str(sample_nonce_err)) {
        i = strlen(sample_nonce_err);
        j = 0;

        while (err_string[i] != ',') {
            sbuf[j] = err_string[i];
            i++;
            j++;
        }

        sbuf[j] = '\0';

        long new_nonce = strtod(sbuf, NULL);
        if (errno == ERANGE)
            RET_ERR("could not convert nonce string to integer value")

        new_nonce += 1;
        api_set_nonce(new_nonce);

        syslog(LOG_INFO, "API Info @ try_fix_error: Error fixed: new nonce set to %li", new_nonce);

        return true;
    }

    /* "no orders" error occur when trying to get order list
     * this is not an actual error */
    if (api_parser_compare_err_str("no orders"))
        return true;

    /* bad status error occur when trying to get status of an order with wrong id
     * this means that order has been already completed or canceled */
    if (api_parser_compare_err_str("bad status"))
        return true;

    syslog(LOG_INFO, "%s: Could not fix error from server (%s)", err_prefix, err_string);
    return false;
}

bool api_get_responce_public(char *url)
{
    const char *err_prefix = "API Error @ api_get_response_public";
    bool res = false;
    CURL *curl;
    CURLcode retcode;

    api_parser_init();

    curl = curl_easy_init();
    if (!curl)
        RET_ERR("could not create CURL object")

    /* set curl options */
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_callback);

    /* perform curl query */
    if ((retcode = curl_easy_perform(curl)) == CURLE_OK) {
        if (api_parser_done()) {
            res = true;
        } else {
            syslog(LOG_ERR, "%s: data is corrupted", err_prefix);
            api_parser_log_last_response();
        }
    } else {
        syslog(LOG_ERR, "%s: curl_easy_perform() failed. Error is %s", err_prefix, curl_easy_strerror(res));
    }

    /* cleanup */
    curl_easy_cleanup(curl);

    return res;
}

bool api_get_responce_private(char *secret, char *key, char *method)
{
    const char *err_prefix = "API Error @ api_get_response_private";
    bool res = false;

    CURL *curl;
    CURLcode retcode;

    char *ct_header = "Content-Type: application/x-www-form-urlencoded";
    char post_params[API_SMALL_BUF_LEN];
    char key_header[API_SMALL_BUF_LEN];
    char sign_header[MAC_STR_LEN];
    int nonce = api_get_nonce();

    char error_message[API_SMALL_BUF_LEN];

    bool success;

    api_parser_init();

    sprintf(post_params, "nonce=%i&method=%s", nonce, method);
    sprintf(key_header, "Key: %s", key);
    api_get_sign_str(secret, post_params, sign_header);

    curl = curl_easy_init();
    if (curl) {
        /* set curl options */
        //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        curl_easy_setopt(curl, CURLOPT_URL, "https://btc-e.com/tapi");
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_callback);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_params);

        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, ct_header);
        headers = curl_slist_append(headers, key_header);
        headers = curl_slist_append(headers, sign_header);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        /* perform curl query */
        if ((retcode = curl_easy_perform(curl)) == CURLE_OK) {
            if (api_parser_done() && api_parser_get_success(&success)) {
                if (success) {
                    res = true;
                } else {
                    if (api_parser_get_err_string(error_message, API_SMALL_BUF_LEN)) {
                        if (try_fix_error(error_message))
                            res = true;
                    } else {
                        syslog(LOG_ERR, "%s: could not get error string from server", err_prefix);
                        api_parser_log_last_response();
                    }
                }
            } else {
                syslog(LOG_ERR, "%s: server return corrupted data", err_prefix);
                api_parser_log_last_response();
            }
        } else {
            syslog(LOG_ERR, "%s: curl_easy_perform() return error (%s)", err_prefix, curl_easy_strerror(res));
        }

        /* cleanup */
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    } else {
        syslog(LOG_ERR, "%s: could not create CURL object", err_prefix);
    }

    return res;
}
