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

static int get_topics_amount(struct uci_package *pkg)
{
    int count = 0;
    struct uci_element *it = NULL;
    uci_foreach_element(&pkg->sections, it) {
        struct uci_section *s = uci_to_section(it);
        if (strcmp(s->type, "topic") == 0) ++count;
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

    conf->topics_amount = get_topics_amount(pkg);
    conf->topics = (topic*) malloc(sizeof(topic) * conf->topics_amount);
    if (!conf->topics) {
        syslog(LOG_ERR, "Failed to allocate topics!");
        goto end;
    }

    int i = 0;
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
    }

end:
    uci_free_context(ctx);
    return conf;
}

void cleanup_config(config *conf)
{
    for (int i = 0; i < conf->topics_amount; i++) {
        FREE(conf->topics[i].topic, conf->topics[i]);
    }

    FREE(
        conf->topics,
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
