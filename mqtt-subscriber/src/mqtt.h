#pragma once

int init_mqtt(struct mosquitto **mosq, config *conf);
void cleanup_mqtt(struct mosquitto **mosq);
