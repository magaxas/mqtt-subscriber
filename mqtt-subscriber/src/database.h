#pragma once

#define DB_PATH "/var/lib/mqttsub.db"

int db_connect();
int db_insert_msg(char *topic, char *msg);
int db_disconnect();
