#include "main.h"

void handle_signal(int sig, siginfo_t *siginfo, void *context)
{
    syslog(LOG_INFO, "Stopping daemon...");
    putchar('q');
}

int main(void)
{
    openlog("mqtt-subscriber", LOG_CONS | LOG_PERROR, LOG_DAEMON);
    syslog(LOG_INFO, "Started mqtt subscriber...");

    config *conf = init_config(conf);
    struct mosquitto *mosq = NULL;
    struct sigaction act;

    memset(&act, '\0', sizeof(act));
    act.sa_sigaction = &handle_signal;
    act.sa_flags = SA_SIGINFO;

    if (sigaction(SIGINT, &act, NULL) < 0
        || sigaction(SIGTERM, &act, NULL) < 0) {
        syslog(LOG_ERR, "Failed to register sigaction!");
        goto end;
    }
    else if (conf == NULL) goto end;
    else if (db_connect() != SQLITE_OK) goto end;
    else if (init_mqtt(&mosq, conf) != MOSQ_ERR_SUCCESS) goto end; 
    getchar();

end:
    cleanup_mqtt(&mosq);
    db_disconnect();
    cleanup_config(conf);

    syslog(LOG_INFO, "Quitting mqtt subscriber...");
    closelog();
    return 0;
}
