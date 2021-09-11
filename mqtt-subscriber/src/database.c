#include "main.h"

static sqlite3 *db = NULL;

int db_connect()
{
    int rc = sqlite3_open(DB_PATH, &db);
    if (rc != SQLITE_OK) {
        syslog(LOG_ERR, "Cannot open database: %s", sqlite3_errmsg(db));
        goto end;      
    }

    char *query = "\
        CREATE TABLE IF NOT EXISTS messages ( \
            id INTEGER PRIMARY KEY, \
            date DATETIME DEFAULT current_timestamp, \
            topic TEXT NOT NULL, \
            message TEXT NOT NULL \
        ); \
    ";

    char *err = NULL;
    sqlite3_exec(db, query, 0, NULL, err);
    if (err) {
        syslog(LOG_ERR, "SQL create error: %s", err);
        sqlite3_free(err);
    }

end:
    return rc;
}

int db_insert_msg(char *topic, char *msg)
{
    char *query = sqlite3_mprintf("\
        INSERT INTO messages (topic, message) \
        VALUES ('%q', '%q'); \
    ", topic, msg);

    char *err = NULL;
    int rc = sqlite3_exec(db, query, 0, NULL, err);
    if (rc != SQLITE_OK) {
        syslog(LOG_ERR, "SQL insert error: %d", rc);
        sqlite3_free(err);
    } else {
        syslog(
            LOG_INFO,
            "Successfully inserted message into DB: \"%s\"",
            msg
        );
    }

    sqlite3_free(query);
}

int db_disconnect()
{
    int rc = sqlite3_close(db);
    if (rc != SQLITE_OK) {
        syslog("Failed to close database, rc=%d", rc);
    }

    return rc;
}
