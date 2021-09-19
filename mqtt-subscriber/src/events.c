#include "main.h"

static const char *template =
    "Event topic: %s\r\n"
    "Key: %s\r\n"
    "Value to compare: %s\r\n"
    "Recieved value: %s\r\n"
    "\r\n"
    "Recieved JSON: %s\r\n";

static value_type get_json_object_type(json_object *obj, const char *key, char **value)
{
    value_type vt;
    json_type type;
    char output[50] = {0};

    type = json_object_get_type(obj);
    switch (type)
    {
    case json_type_double:
    {
        double dval = json_object_get_double(obj);
        snprintf(output, 50, "%lf", dval);
        *value = strdup(output);
        vt = dec;
        syslog(LOG_DEBUG, "%s is json_type_double, value = %lf", key, dval);
        break;
    }
    case json_type_int:
    {
        int32_t ival = json_object_get_int(obj);
        snprintf(output, 50, "%d", ival);
        *value = strdup(output);
        vt = dec;
        syslog(LOG_DEBUG, "%s is json_type_int, value = %d", key, ival);
        break;
    }
    case json_type_string:
    {
        char *sval = json_object_get_string(obj);
        *value = strdup(sval);
        vt = str;
        syslog(LOG_DEBUG, "%s is json_type_string, value = %s", key, sval);
        break;
    }
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
    if (value1 == NULL || value2 == NULL) {
        syslog(LOG_INFO, "value1 or value2 is null");
        return meets_req;
    }

    switch (vt)
    {
    case dec:
    {
        double val1 = atof(value1), val2 = atof(value2);
        if (ct == EQ) meets_req = val2 == val1;
        else if (ct == NEQ) meets_req = val2 != val1;
        else if (ct == LT) meets_req = val2 < val1;
        else if (ct == LTE) meets_req = val2 <= val1;
        else if (ct == GT) meets_req = val2 > val1;
        else if (ct == GTE) meets_req = val2 >= val1;
        break;
    }
    case str:
    {
        if (ct == EQ) meets_req = strcmp(value2, value2) == 0;
        else if (ct == NEQ) meets_req = strcmp(value2, value1) != 0;
        else if (ct == LT) meets_req = strcmp(value2, value1) < 0;
        else if (ct == LTE) meets_req = strcmp(value2, value1) < 0 || strcmp(value2, value1) == 0;
        else if (ct == GT) meets_req = strcmp(value2, value1) > 0;
        else if (ct == GTE) meets_req = strcmp(value2, value1) > 0 || strcmp(value2, value1) == 0;
        break;
    }
    case nil:
    default:
        meets_req = false;
    }

    return meets_req;
}

int handle_events(config *conf, char *topic, char *payload)
{
    int rc = 0;
    json_object *root = json_tokener_parse(payload);
    if (!root) {
        syslog(LOG_WARNING, "Recieved non-valid json string!");
        return 1;
    }

    for (int i = 0; i < conf->events_amount; i++) {
        if (strcmp(conf->events[i].topic, topic) != 0) continue;
        
        json_object_object_foreach(root, key, val) {
            if (key == NULL || val == NULL) continue;

            char *value = NULL;
            value_type vt = get_json_object_type(val, key, &value);

            if (vt != conf->events[i].type ||
                strcmp(conf->events[i].key, key) != 0 ||
                !meets_req(vt, conf->events[i].ct, conf->events[i].value, value)
            ) {
                FREE(value);
                continue;
            }

            int data_length = (
                strlen(template) +
                strlen(conf->events[i].topic) +
                strlen(conf->events[i].key) +
                strlen(conf->events[i].value) +
                strlen(value) +
                strlen(payload)
            );
            char *data = (char*) calloc(data_length, sizeof(char));
            snprintf(
                data,
                data_length,
                template,
                conf->events[i].topic,
                conf->events[i].key,
                conf->events[i].value,
                value,
                payload
            );

            int rc = send_email(
                conf->events[i].from_email,
                conf->events[i].to_email,
                "New event from MQTT subscriber",
                data,
                conf->events[i].smtp_host,
                conf->events[i].smtp_port,
                conf->events[i].smtp_username,
                conf->events[i].smtp_password,
                conf->events[i].smtp_host
            );

            if (rc != CURLE_OK) {
                syslog(LOG_ERR, "Failed to send email, rc=%d", rc);
            } else {
                syslog(LOG_INFO, "Successfully sent email!");
            }

            FREE(payload);
            FREE(value);
        }
    }

    json_object_put(root);
    return rc;
}
