#include "main.h"

static value_type get_json_object_type(json_object *obj, const char *key, char *value)
{
    value_type vt;
    json_type type;
    char output[50] = {0};

    type = json_object_get_type(obj);
    switch (type)
    {
    case json_type_double:
        double dval = json_object_get_double(obj);
        snprintf(output, 50, "%lf", dval);
        value = strdup(output);
        vt = dec;

        syslog(LOG_DEBUG, "%s is json_type_double", key);
        syslog(LOG_DEBUG, "          value: %lf", dval);
        break;

    case json_type_int:
        int32_t ival = json_object_get_int(obj);
        snprintf(output, 50, "%d", ival);
        value = strdup(output);
        vt = dec;

        syslog(LOG_DEBUG, "%s is json_type_int\n", key);
        syslog(LOG_DEBUG, "          value: %d\n", ival);
        break;

    case json_type_string:
        char *sval = json_object_get_string(obj);
        value = strdup(sval);
        vt = str;

        syslog(LOG_DEBUG, "%s is json_type_string\n", key);
        syslog(LOG_DEBUG, "          value: %s\n", sval);
        break;
    default:
        vt = nil;
        syslog(LOG_DEBUG, "%s is null\n", key);
        break;
    }

    return vt;
}

static bool meets_req(value_type vt, comparison_type ct, char *value1, char *value2)
{
    bool meets_req = false;

    switch (vt)
    {
    case dec:
        int val1 = atof(value1), val2 = atof(value2);
        if (ct == EQ) meets_req = val1 == val2;
        else if (ct == NEQ) meets_req = val1 != val2;
        else if (ct == LT) meets_req = val1 < val2;
        else if (ct == LTE) meets_req = val1 <= val2;
        else if (ct == GT) meets_req = val1 > val2;
        else if (ct == GTE) meets_req = val1 >= val2;
        break;

    case str:
        if (ct == EQ) meets_req = strcmp(val1, val2) == 0;
        else if (ct == NEQ) meets_req = strcmp(val1, val2) != 0;
        else if (ct == LT) meets_req = strcmp(val1, val2) < 0;
        else if (ct == LTE) meets_req = strcmp(val1, val2) < 0 || strcmp(val1, val2) == 0;
        else if (ct == GT) meets_req = strcmp(val1, val2) > 0;
        else if (ct == GTE) meets_req = strcmp(val1, val2) > 0 || strcmp(val1, val2) == 0;
        break;
    }

    return meets_req;
}

int handle_events(config *conf, char *topic, char *payload)
{
    int rc = 0;
    struct json_object_iterator it;
    struct json_object_iterator itEnd;

    json_object *root = json_tokener_parse(payload);
    if (!root) {
        syslog(LOG_WARNING, "Recieved non-valid json string!");
        return 1;
    }

    it = json_object_iter_init_default();
    it = json_object_iter_begin(root);
    itEnd = json_object_iter_end(root);

    for (int i = 0; i < conf->events_amount; i++) {
        if (strcmp(conf->events[i].topic, topic) != 0) continue;

        while (!json_object_iter_equal(&it, &itEnd))
        {
            char *value = NULL;
            const char* key = json_object_iter_peek_name(&it);
            json_object* val = json_object_iter_peek_value(&it);
            value_type vt = get_json_object_type(val, key, value);

            syslog(LOG_DEBUG, "%s  -> %s\n", key, json_object_get_string(val));
            if (vt != conf->events[i].type || 
                strcmp(conf->events[i].key, key) != 0 ||
                !meets_req(vt, conf->events[i].ct, conf->events[i].value, value)
            ) {
                FREE(value);
                continue;
            }
            
            int rc = send_email(
                conf->events[i].from_email,
                conf->events[i].to_email,
                "New event from MQTT subscriber",
                "demo payload", //TODO: set payload
                conf->events[i].smtp_host,
                conf->events[i].smtp_port,
                conf->events[i].smtp_username,
                conf->events[i].smtp_password,
                conf->events[i].smtp_host
            );

            if (rc != CURLE_OK) {
                syslog(LOG_ERR, "Failed to send email, rc=%d", rc);
            }

            json_object_iter_next(&it);
            FREE(value);
        }
    }

    json_object_put(root);
    return rc;
}
