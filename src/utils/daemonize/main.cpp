#include <iostream>
#include "daemonize.h"

int APP_RUNNING = 0;
std::string APP_NAME;
std::string PID_FILE = "/tmp/kraken.pid";

int load_config(const std::string &config_file) {
    syslog(LOG_INFO, "reload the krakens behaviour");
    return 0;
}

int main() {
    std::cout << "Hello, World, prepare for the daemon spamming your syslog!" << std::endl;

    daemonize();

    signal(SIGINT, handle_signal);
    signal(SIGHUP, handle_signal);

    openlog("daemonize example", LOG_PID | LOG_CONS, LOG_DAEMON);

    int i = 0;
    while(m_timer.wait_for(std::chrono::seconds(10))) {
        if(!APP_RUNNING) break;
        syslog(LOG_INFO, "release the kraken %i", ++i);
    }

    closelog();
    unlink(PID_FILE.c_str());



    return 0;
}
