//
// Created by ztk on 2021-07-17.
//

#ifndef MYSQLGEAR_CONFIG_H
#define MYSQLGEAR_CONFIG_H

#include <vector>
#include <string>
#include "mysqlserver.h"

struct Config {

    std::string app_name = "mysqlgeard";

    bool rundaemonized = false;

    //! PID file when running daemonized
    std::string pid_file = "/usr/local/run/ztk/mysqlgeard.pid";

    //! output log file for stuff
    std::string error_log = "/var/log/mysqlgeard/output.log";

    bool gear_server_enable = true;

    bool obey_work_retry_count = false;

    int gear_server_port = 1370;

    std::string gear_server_host = "127.0.0.1";

    int gear_server_backlog = 500;

    int gear_server_rcv_timeout_sec = 3;
    int gear_server_rcv_timeout_usec = 0;

    int gear_server_snd_timeout_sec = 3;
    int gear_server_snd_timeout_usec = 0;

    int gear_server_max_connections = 20;

    bool gear_bind_network_device = false;

    std::string gear_network_device = "eth0";

    int work_retry_count = 3;

    std::vector<MysqlServer> mysql_server;

    bool syslogOutput = false;

    int mysql_initialconnections = 6;
    int mysql_maxconnections = 50;


};

#endif //MYSQLGEAR_CONFIG_H
