#include <iostream>
#include <fstream>
#include <sstream>

#include "utils/daemonize/daemonize.h"
#include "config.h"
#include "input/InputHandler.h"
#include "output/OutputHandler.h"
#include "MysqlGearQueueEntry.h"

Config config{};

static bool rundaemonized = false;
static FILE *log_stream;
std::string PID_FILE;
std::string APP_NAME = "MysqlGear";
int APP_RUNNING = 0;

int8_t PROTO_VERSION=1;

int load_config(const std::string &config_file_name) {
    // TODO check file exists and file exists in $rundir and /etc/ path
    std::ifstream fin(config_file_name);
    std::string line;

    config.mysql_server.clear();
    config.mysql_server.resize(0);

    while (getline(fin, line)) {

        std::istringstream sin(line.substr(line.find('=') + 1));

        if (line.find("app_name") != std::string::npos)
        {
            std::string old_name = config.app_name;
            sin >> config.app_name;
            syslog(LOG_INFO, "%s: is now called %s", old_name.c_str(), config.app_name.c_str());
            APP_NAME = config.app_name;
        } else if (line.find("pid_file") != std::string::npos)
        {
            sin >> config.pid_file;
            PID_FILE = config.pid_file;
        }

        else if (line.find("gear_server_port") != std::string::npos)
            sin >> config.gear_server_port;
        else if (line.find("gear_server_host") != std::string::npos)
            sin >> config.gear_server_host;
        else if (line.find("gear_server_backlog") != std::string::npos)
            sin >> config.gear_server_backlog;
        else if (line.find("gear_network_device") != std::string::npos)
            sin >> config.gear_network_device;
        else if (line.find("bind_network_device") != std::string::npos)
            sin >> config.gear_bind_network_device;

        else if( line.find("gear_server_rcv_timeout_sec") != std::string::npos )
            sin >> config.gear_server_rcv_timeout_sec;
        else if( line.find("gear_server_rcv_timeout_usec") != std::string::npos )
            sin >> config.gear_server_rcv_timeout_usec;

        else if( line.find("gear_server_snd_timeout_sec") != std::string::npos )
            sin >> config.gear_server_snd_timeout_sec;
        else if( line.find("gear_server_snd_timeout_usec") != std::string::npos )
            sin >> config.gear_server_snd_timeout_usec;

        else if( line.find("gear_server_max_connections") != std::string::npos )
            sin >> config.gear_server_max_connections;



        else if (line.find("work_retry_count") != std::string::npos)
            sin >> config.work_retry_count;
        else if (line.find("obey_work_retry_count") != std::string::npos)
            sin >> config.obey_work_retry_count;
        else if (line.find("mysql_server") != std::string::npos) {
            MysqlServer mysqlServer{};
            sin >> mysqlServer.connection_string;
            config.mysql_server.push_back(mysqlServer);
        }

        else if(line.find("mysql_initialconnections") != std::string::npos)
            sin >> config.mysql_initialconnections;
        else if(line.find("mysql_maxconnections") != std::string::npos)
            sin >> config.mysql_maxconnections;

    }
#ifdef DEBUG
    std::cout << "loaded config with " << config.mysql_server.size() << " database server strings\n";
#endif

    return 1;
}

void shutdownApp() {
    {
        APP_RUNNING = 0;
        m_timer.interrupt();
    }
}

int main(int argc, char *argv[]) {
#ifdef DEBUG
    std::cout << "Hello, World!" << std::endl;
#endif
    int opt;
    bool config_loaded = false;

    while ((opt = getopt(argc, argv, ":c:d")) != -1) {
        switch (opt) {
            case 'c':
                //configuration file
                conf_file_name = optarg;
                config_loaded = load_config(conf_file_name);
                break;
            case 'd':
                if (config_loaded) {
                    syslog(LOG_INFO, "%s: run daemonized", APP_NAME.c_str());
                    config.rundaemonized = true;
                    rundaemonized = true;
                } else {
                    std::cout << "please specify -c option first\n";
                    exit(EXIT_FAILURE);
                }
                break;
            default:
                break;
        }
    }

    if (config.rundaemonized) {
        rundaemonized = true;
        daemonize();
    } else {
        APP_RUNNING = 1;
    }


/* Open system log and write message to it */
//    openlog(app_name.c_str(), LOG_PID|LOG_CONS, LOG_DAEMON);
//    syslog(LOG_INFO, "Started %s", app_name.c_str());
    openlog(APP_NAME.c_str(), LOG_PID | LOG_CONS, LOG_DAEMON);
    syslog(LOG_INFO, "Started %s", APP_NAME.c_str());

    if (rundaemonized) {
        /* Daemon will handle two signals */
        signal(SIGINT, handle_signal);
        signal(SIGHUP, handle_signal);
    }

    /*
     * Application logic
     */


    utils::LockedQueue<MysqlGearQueueEntry> messages;
    LibzdbHandler databaseHandler(&config);
    ThreadpoolHandler inputNetworkHandlersThreadpool;
    ThreadpoolHandler databaseTasksThreadpool;

    databaseTasksThreadpool.initialize(config.mysql_maxconnections+1);

    InputHandler inputHandler(&config, &messages, &databaseHandler, &databaseTasksThreadpool);
    inputHandler.initialize();

    auto outputHandler = OutputHandler(&config, &messages, &databaseHandler, &databaseTasksThreadpool);
    std::thread outputHandlerThread = outputHandler.run();




    while (m_timer.wait_for(std::chrono::seconds(10))) {
        if (!APP_RUNNING) break;

    }

    inputHandler.deinitialize();

    outputHandler.stop();
    if(outputHandlerThread.joinable()) {
        outputHandlerThread.join();
    }

    syslog(LOG_CRIT, "could not queue sql:");
    while(!messages.empty()) {
        auto msg = messages.pop();
        syslog(LOG_CRIT, "%s", msg.query.c_str());
    }

    /* Close log file, when it is used. */
    if (log_stream != stdout && log_stream != nullptr) {
        fclose(log_stream);
    }
    /* Write system log and close it. */
    //syslog(LOG_INFO, "Stopped %s", app_name.c_str());
    syslog(LOG_INFO, "Stopped %s", APP_NAME.c_str());
    closelog();
    if (rundaemonized) {
        unlink(PID_FILE.c_str());
    }
    return EXIT_SUCCESS;
}
