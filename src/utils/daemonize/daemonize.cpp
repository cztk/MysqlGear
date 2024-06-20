//
// Created by zysik on 17.11.20.
//

#include "daemonize.h"

void handle_signal(int sig) {
    if (sig == SIGINT) {
        syslog(LOG_INFO, "%s: daemonize shutting down", APP_NAME.c_str());

        /* Unlock and close lockfile */
        if (pid_fd != -1) {
            lockf(pid_fd, F_ULOCK, 0);
            close(pid_fd);
        }
        /* Try to delete lockfile */
//        if (app_pid_file != NULL) {
//            unlink(app_pid_file);
        unlink(PID_FILE.c_str());
//        }
        APP_RUNNING = 0;
        m_timer.interrupt();
        /* Reset signal handling to default behavior */
        signal(SIGINT, SIG_DFL);
    } else if (sig == SIGHUP) {
        syslog(LOG_INFO, "%s: daemonize reload config", APP_NAME.c_str());
        //fprintf(log_stream, "Debug: reloading daemon config file ...\n");
        // TODO notify main_thread about change setConfig-chain
        load_config(conf_file_name);
    } else if (sig == SIGCHLD) {
        syslog(LOG_INFO, "%s: daemonize received SIGCHLD signal", APP_NAME.c_str());
        //fprintf(log_stream, "Debug: received SIGCHLD signal\n");
    }
}

/**
 * \brief This function will daemonize this app
 */
void daemonize() {
    pid_t pid = 0;
    int fd;

    /* Fork off the parent process */
    pid = fork();

    /* An error occurred */
    if (pid < 0) {
        syslog(LOG_INFO, "%s: daemonize: pid < 0\n", APP_NAME.c_str());
        exit(EXIT_FAILURE);
    }

    /* Success: Let the parent terminate */
    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }

    /* On success: The child process becomes session leader */
    if (setsid() < 0) {
        syslog(LOG_INFO, "%s: daemonize: setsid() < 0\n", APP_NAME.c_str());

        exit(EXIT_FAILURE);
    }

    /* Ignore signal sent from child to parent process */
    signal(SIGCHLD, SIG_IGN);

    /* Fork off for the second time*/
    pid = fork();

    /* An error occurred */
    if (pid < 0) {
        syslog(LOG_INFO, "%s: daemonize: pid < 0\n", APP_NAME.c_str());
        exit(EXIT_FAILURE);
    }

    /* Success: Let the parent terminate */
    if (pid > 0) {
        syslog(LOG_INFO, "%s: Success: Let the parent terminate\n", APP_NAME.c_str());
        exit(EXIT_SUCCESS);
    }

    /* Set new file permissions */
    umask(0);

    /* Change the working directory to the root directory */
    /* or another appropriated directory */
    if (0 != chdir("/")) {
        syslog(LOG_INFO, "%s: daemonize: could not change directory to /\n", APP_NAME.c_str());
        exit(EXIT_FAILURE);
    }

    /* Close all open file descriptors */
    for (fd = sysconf(_SC_OPEN_MAX); fd > 0; fd--) {
        close(fd);
    }

    /* Reopen stdin (fd = 0), stdout (fd = 1), stderr (fd = 2) */
    stdin = fopen("/dev/null", "r");
    stdout = fopen("/dev/null", "w+");
    stderr = fopen("/dev/null", "w+");

    /* Try to write PID of daemon to lockfile */
    //if (!app_config->app_pid_file.empty())
    //{
    char str[256];
    //pid_fd = open(app_config->app_pid_file.c_str(), O_RDWR|O_CREAT, 0640);
    pid_fd = open(PID_FILE.c_str(), O_RDWR | O_CREAT, 0640);

    if (pid_fd < 0) {
        /* Can't open lockfile */
        syslog(LOG_INFO, "%s: daemonize: Can't open lockfile '%s'\n", APP_NAME.c_str(), PID_FILE.c_str());
        exit(EXIT_FAILURE);
    }
    if (lockf(pid_fd, F_TLOCK, 0) < 0) {
        /* Can't lock file */
        syslog(LOG_INFO, "%s: daemonize: Can't lock file '%s'\n", APP_NAME.c_str(), PID_FILE.c_str());
        exit(EXIT_FAILURE);
    }
    /* Get current PID */
    sprintf(str, "%d\n", getpid());
    /* Write PID to lockfile */
    if (0 >= write(pid_fd, str, strlen(str))) {
        syslog(LOG_INFO, "%s: daemonize: Write PID to lockfile failed file: '%s' pid: %s\n", APP_NAME.c_str(),
               PID_FILE.c_str(), str);

    }
    //}
    APP_RUNNING = 1;
}