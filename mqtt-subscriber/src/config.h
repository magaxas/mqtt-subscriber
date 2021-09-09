#pragma once

#define UCI_CONFIG_NAME "mqttsub"

typedef struct {
    int qos;
    char *topic;
} topic;

typedef struct {
    int topics_amount;
    topic *topics;

    char *host;
    int port;
    char *username;
    char *password;

    //Security settings
    bool use_tls_ssl;
    char *tls_type;
    char *psk;
    char *identity;
    char *ca_file;
    char *cert_file;
    char *key_file;
} config;

config *init_config();
void cleanup_config(config *conf);
