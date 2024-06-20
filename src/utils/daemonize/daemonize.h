//
// Created by zysik on 17.11.20.
//

#ifndef DAEMONIZE_DAEMONIZE_H
#define DAEMONIZE_DAEMONIZE_H

/// @cond
#include <csignal>
#include <syslog.h>
#include <string>
#include <fcntl.h>
#include <zconf.h>
#include <sys/stat.h>
#include <cstring>
#include "../timedwaiter/timed_waiter.h"
/// @endcond

static int pid_fd = -1;
extern int APP_RUNNING;
extern std::string APP_NAME;
extern std::string PID_FILE;
static timed_waiter m_timer;
static std::string conf_file_name = "";

void daemonize();

/**
 * \brief Callback function for handling signals.
 * \param	sig	identifier of signal
 */
void handle_signal(int sig);

extern int load_config(const std::string &);


#endif //DAEMONIZE_DAEMONIZE_H
