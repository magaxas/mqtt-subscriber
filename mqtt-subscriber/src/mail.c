#include "main.h"

struct upload_status {
    char *to;
    char *from;
    char *subject;
    char *data;
    size_t bytes_read;
};

#define PARAMS 5
static const char *template =
    "Date: %s\r\n"
    "To: %s\r\n"
    "From: %s\r\n"
    "Subject: %s\r\n"
    "\r\n" //empty line to divide headers from body
    "%s\r\n";

static size_t payload_source(char *ptr, size_t size, size_t nmemb, void *userp)
{
    struct upload_status *upload = (struct upload_status*) userp;
    char *data;
    size_t room = size * nmemb;

    data = malloc(
        strlen(template) +
        strlen(upload->to) +
        strlen(upload->from) +
        strlen(upload->subject) +
        strlen(upload->data) -
        (PARAMS*2 - 1)
    );

    if (!data || size == 0 || nmemb == 0 || size*nmemb < 1) return 0;

    sprintf(
        data,
        template,
        __DATE__,
        upload->to,
        upload->from,
        upload->subject,
        upload->data
    );

    if (data) {
        size_t len = strlen(data);
        if (room < len) len = room;
        
        memcpy(ptr, data, len);
        upload->bytes_read += len;
        return len;
    }

    return 0;
}

int send_email(
    char *from,
    char *to,
    char *subject,
    char *payload,
    char *server,
    int port,
    char *username,
    char *password,
    bool use_ssl)
{
    CURL *curl;
    CURLcode res = CURLE_OK;
    struct curl_slist *recipients;
    struct upload_status upload = {
        .to = to,
        .from = from,
        .subject = subject,
        .data = payload,
        .bytes_read = 0
    };

    curl = curl_easy_init();
    if (curl) {
        char url[1024] = {0};
        sprintf("smtps://%s:%d", server, port);

        curl_easy_setopt(curl, CURLOPT_USERNAME, username);
        curl_easy_setopt(curl, CURLOPT_PASSWORD, password);
        curl_easy_setopt(curl, CURLOPT_URL, url);

        if (use_ssl) {
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        }

        curl_easy_setopt(curl, CURLOPT_MAIL_FROM, from);
        recipients = curl_slist_append(recipients, to);
        curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

        curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
        curl_easy_setopt(curl, CURLOPT_READDATA, &upload);
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            syslog(
                LOG_ERR,
                "curl_easy_perform() failed: %s",
                curl_easy_strerror(res)
            );
        }

        curl_slist_free_all(recipients);
        curl_easy_cleanup(curl);
    }

    return (int) res;
}
