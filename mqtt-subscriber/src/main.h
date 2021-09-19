#pragma once

#define RECONNECT_INTERVAL 5

//System includes
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <stdbool.h>
#include <signal.h>
#include <string.h>
#include <stdarg.h>

//Libraries
#include <sqlite3.h>
#include <mosquitto.h>
#include <uci.h>
#include <curl/curl.h>
#include <json-c/json.h>

//Source includes
#include "utils.h"
#include "config.h"
#include "database.h"
#include "mqtt.h"
#include "mail.h"
#include "events.h"
