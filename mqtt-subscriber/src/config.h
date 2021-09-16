#pragma once

#define UCI_CONFIG_NAME "mqttsub"

typedef enum { EQ, NEQ, GT, GTE, LT, LTE } comparison_type;
typedef enum { nil, str, dec } value_type;

typedef struct {
    int qos;
    char *topic;
} topic;

typedef struct {
    char *topic;
    char *key;
    value_type type;
    comparison_type ct;
    char *value;

    //Email settings
    char *smtp_host;
    int smtp_port;
    char *smtp_username;
    char *smtp_password;
    bool smtp_use_ssl;

    char *to_email;
    char *from_email;
} event;

typedef struct {
    int topics_amount;
    topic *topics;

    int events_amount;
    event *events;

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
