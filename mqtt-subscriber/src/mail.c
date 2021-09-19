#include "main.h"

struct upload_status {
    char *data;
    size_t bytes_read;
};

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
    const char *data;
    size_t room = size * nmemb;

    if (size == 0 || nmemb == 0 || size*nmemb < 1) return 0;
    data = &(upload->data)[upload->bytes_read];

    if (data) {
        size_t len = strlen(data);
        if (room < len) len = room;
        
        memcpy(ptr, data, len);
        upload->bytes_read += len;
        return len;
    }

    return 0;
}

static int get_int_length(int num) {
    int count = 0;
    do {
        count++;
        num /= 10;
    } while(num != 0);

    return count;
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
    bool use_ssl
) {
    CURL *curl;
    CURLcode res = CURLE_OK;

    struct curl_slist *recipients = NULL;
    struct upload_status upload = {0};

    int url_length = strlen(server) + get_int_length(port) + 10;
    char *url = (char*) calloc(
        strlen(server) + get_int_length(port) + 10,
        sizeof(char)
    );

    int data_length = (
        strlen(template) +
        strlen(to) +
        strlen(from) +
        strlen(subject) +
        strlen(payload)
    );
    char *data = (char*) calloc(data_length, sizeof(char));
    snprintf(
        data,
        data_length,
        template,
        __DATE__,
        to,
        from,
        subject,
        payload
    );

    if (url == NULL || data == NULL) {
        syslog(LOG_ERR, "Failed to allocate url or data!");
        return CURLE_OUT_OF_MEMORY;
    }

    curl = curl_easy_init();
    if (curl) {
        upload.bytes_read = 0;
        upload.data = data;
        snprintf(url, url_length, "smtp://%s:%d", server, port);

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
                LOG_ERR, "curl_easy_perform() failed: %d", (int) res
            );
        }

        curl_slist_free_all(recipients);
        curl_easy_cleanup(curl);
    }
    FREE(url);
    FREE(data);

    return (int) res;
}
