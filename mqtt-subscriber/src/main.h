#pragma once

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

//Source includes
#include "utils.h"
#include "config.h"
#include "database.h"
#include "mqtt.h"
