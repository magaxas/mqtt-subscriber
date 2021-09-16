#include "main.h"

void on_connect(struct mosquitto *mosq, void *obj, int rc)
{
    if (rc) {
        syslog(LOG_ERR, "Error on_connect, with rc=%d\n", rc);
        return;
    }

    config *conf = (config*) obj;
    if (conf == NULL) {
        syslog(LOG_ERR, "Empty config from on_connect callback!");
        return;
    }

    for (int i = 0; i < conf->topics_amount; i++) {
        if (mosquitto_subscribe(
                mosq,
                NULL,
                conf->topics[i].topic,
                conf->topics[i].qos
        ) == MOSQ_ERR_SUCCESS) {
            syslog(
                LOG_INFO,
                "Successfully subscribed to the \"%s\"",
                conf->topics[i].topic
            );
        }
        else {
            syslog(
                LOG_INFO,
                "Failed to subscribe to the \"%s\"",
                conf->topics[i].topic
            );
        }
    }
}

void on_message(
    struct mosquitto *mosq,
    void *obj,
    const struct mosquitto_message *msg)
{
    if (msg->topic && msg->payload && strlen(msg->topic) > 0 && strlen(msg->payload) > 0) {
        syslog(
            LOG_INFO,
            "New message with topic %s: %s", msg->topic,
            (char*) msg->payload
        );

        db_insert_msg(msg->topic, (char*) msg->payload);

        config *conf = (config*) obj;
        if (conf == NULL) {
            syslog(LOG_ERR, "Empty config from on_message callback!");
            return;
        }

        handle_events(conf, msg->topic, msg->payload);
    }
    else {
        syslog(LOG_WARNING, "Received empty message!");
    }
}

int init_mqtt(struct mosquitto **mosq, config *conf)
{
    //Quit if most basic configuration is not valid
    if (!conf->host || !conf->port) return 1;

    int rc = mosquitto_lib_init();
    if (rc) syslog(LOG_ERR, "Could not init mosquito lib with rc=%d\n", rc);

    *mosq = mosquitto_new("mqttsub", false, conf);
    mosquitto_connect_callback_set(*mosq, on_connect);
    mosquitto_message_callback_set(*mosq, on_message);

    if (conf->username || conf->password) {
        rc = mosquitto_username_pw_set(*mosq, conf->username, conf->password);
    }

    if (conf->use_tls_ssl && conf->tls_type) {
        if (strcmp(conf->tls_type, "psk") == 0 && conf->psk && conf->identity) {
            rc = mosquitto_tls_psk_set(
                *mosq,
                conf->psk,
                conf->identity,
                NULL
            );
        }
        else if (strcmp(conf->tls_type, "cert") == 0 && 
            conf->ca_file &&
            conf->cert_file &&
            conf->key_file
        ) {
            rc = mosquitto_tls_set(
                *mosq,
                conf->ca_file,
                NULL,
                conf->cert_file,
                conf->key_file,
                NULL
            );
        }
    }

    if (mosquitto_connect(*mosq, conf->host, conf->port, 10) != MOSQ_ERR_SUCCESS) {
        syslog(LOG_ERR, "Could not conect to broker with rc=%d\n", rc);
    }

    return rc;
}

void cleanup_mqtt(struct mosquitto **mosq)
{
    if (!*mosq) return;
    mosquitto_loop_stop(*mosq, true);
    mosquitto_disconnect(*mosq);
    mosquitto_destroy(*mosq);
    mosquitto_lib_cleanup();
}
