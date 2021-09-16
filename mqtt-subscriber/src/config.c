#include "main.h"

static int init_uci(
    struct uci_context **ctx,
    struct uci_package **pkg)
{
    *ctx = uci_alloc_context();

    if (*ctx == NULL) {
        syslog(LOG_ERR, "Failed to allocate uci!");
        return 1;
    }
    else if (uci_load(*ctx, UCI_CONFIG_NAME, pkg) != UCI_OK) {
        syslog(LOG_ERR, "Failed to load uci!");
        return 2;
    }

    return UCI_OK;
}

static int get_amount(struct uci_package *pkg, char *sct)
{
    int count = 0;
    struct uci_element *it = NULL;
    uci_foreach_element(&pkg->sections, it) {
        struct uci_section *s = uci_to_section(it);
        if (strcmp(s->type, sct) == 0) ++count;
    }

    return count;
}

static char *get_string_option(
    struct uci_context *ctx,
    struct uci_section *sct,
    char *name)
{
    const char *opt = uci_lookup_option_string(ctx, sct, name);
    return opt != NULL ? strdup(opt) : NULL;
}

static int *get_integer_option(
    struct uci_context *ctx,
    struct uci_section *sct,
    char *name)
{
    char *tmp = get_string_option(ctx, sct, name);
    if (tmp != NULL) {
        int num = atoi(tmp);
        free(tmp);
        return num;
    }

    return -1;
}

config *init_config()
{
    config *conf = (config*) malloc(sizeof(config));
    struct uci_context *ctx = NULL;
    struct uci_package *pkg = NULL;
    struct uci_element *el = NULL;

    if (conf == NULL || init_uci(&ctx, &pkg) != UCI_OK) {
        syslog(LOG_ERR, "Failed to init config!");
        goto end;
    }

    conf->topics_amount = get_amount(pkg, "topic");
    conf->topics = (topic*) malloc(sizeof(topic) * conf->topics_amount);
    if (!conf->topics) {
        syslog(LOG_ERR, "Failed to allocate topics!");
        goto end;
    }

    conf->events_amount = get_amount(pkg, "event");
    conf->events = (event*) malloc(sizeof(event) * conf->events_amount);
    if (!conf->events) {
        syslog(LOG_ERR, "Failed to allocate events!");
        goto end;
    }

    int i = 0, j = 0;
    struct uci_element *tmp = NULL;
    uci_foreach_element(&pkg->sections, tmp) {
        struct uci_section *sct = uci_to_section(tmp);
        if (strcmp(sct->type, "mqttsub") == 0) {
            conf->host = get_string_option(ctx, sct, "host");
            conf->port = get_integer_option(ctx, sct, "port");

            conf->username = get_string_option(ctx, sct, "username");
            conf->password = get_string_option(ctx, sct, "password");

            //Security settings
            conf->tls_type = get_string_option(ctx, sct, "tls_type");
            conf->psk = get_string_option(ctx, sct, "psk");
            conf->identity = get_string_option(ctx, sct, "identity");
            conf->ca_file = get_string_option(ctx, sct, "ca_file");
            conf->cert_file = get_string_option(ctx, sct, "cert_file");
            conf->key_file = get_string_option(ctx, sct, "key_file");
        }
        else if (strcmp(sct->type, "topic") == 0) {
            conf->topics[i].topic = get_string_option(ctx, sct, "topic");
            conf->topics[i].qos = get_integer_option(ctx, sct, "qos");
            ++i;
        }
        else if (
            strcmp(sct->type, "event") == 0 && 
            get_integer_option(ctx, sct, "enabled") == 1
        ) {
            conf->events[j].topic = get_string_option(ctx, sct, "value");
            conf->events[j].key = get_string_option(ctx, sct, "key");
            conf->events[j].type = get_integer_option(ctx, sct, "type");
            conf->events[j].ct = get_integer_option(ctx, sct, "ct");
            conf->events[j].value = get_string_option(ctx, sct, "value");

            //Email settings
            conf->events[j].smtp_host = get_string_option(ctx, sct, "smtp_host");
            conf->events[j].smtp_port = get_integer_option(ctx, sct, "smtp_port");
            conf->events[j].smtp_username = get_string_option(ctx, sct, "smtp_username");
            conf->events[j].smtp_password = get_integer_option(ctx, sct, "smtp_password");
            conf->events[j].smtp_use_ssl = get_string_option(ctx, sct, "smtp_use_ssl");
            conf->events[j].from_email = get_integer_option(ctx, sct, "from_email");
            conf->events[j].to_email = get_string_option(ctx, sct, "to_email");
            ++j;
        }
    }

end:
    uci_free_context(ctx);
    return conf;
}

void cleanup_config(config *conf)
{
    int i;
    for (i = 0; i < conf->topics_amount; i++) {
        FREE(conf->topics[i].topic, conf->topics[i]);
    }

    for (i = 0; i < conf->events; i++) {
        FREE(
            conf->events[i].topic,
            conf->events[i].key,
            conf->events[i].type,
            conf->events[i].ct,
            conf->events[i].value,
            conf->events[i].smtp_host,
            conf->events[i].smtp_port,
            conf->events[i].smtp_username,
            conf->events[i].smtp_password,
            conf->events[i].smtp_use_ssl,
            conf->events[i].from_email,
            conf->events[i].to_email
        );
    }

    FREE(
        conf->topics,
        conf->events,
        conf->host,
        conf->username,
        conf->password,
        conf->tls_type,
        conf->psk,
        conf->identity,
        conf->ca_file,
        conf->cert_file,
        conf->key_file,
        conf
    );
}
