#pragma once

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
);
