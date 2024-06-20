//
// Created by ztk on 2021-08-02.
//

#include "GearServer.h"


GearServer::GearServer(Config *pConfig, utils::LockedQueue<MysqlGearQueueEntry> *pQueue, LibzdbHandler *pDatabasehandler, ThreadpoolHandler *pDatabaseTasksThreadpool) :
stop_(false), config(pConfig), messages(pQueue), databasehandler(pDatabasehandler), databaseTasksThreadpool(pDatabaseTasksThreadpool) {

}

GearServer::~GearServer() = default;

void GearServer::initialize() {

    server.setup(config->gear_network_device.c_str(), config->gear_server_host.c_str(), config->gear_server_port, config->gear_bind_network_device, config->gear_server_backlog);

}

void GearServer::stop() {
    syslog(LOG_INFO, "GearServer stop requested");
    {
        stop_ = true;
        m_timer.interrupt();
    }
}

void GearServer::monitor_tasks(const thread_pool *pool, synced_stream *sync_out)
{
    sync_out->println(pool->get_tasks_total(),
                      " tasks total, ",
                      pool->get_tasks_running(),
                      " tasks running, ",
                      pool->get_tasks_queued(),
                      " tasks queued.");
}

void GearServer::thread_main() {
    //TODO conditional events
    thread_pool client_threadpool(config->gear_server_max_connections);
    struct timeval rcv_timeout;
    struct timeval snd_timeout;
    rcv_timeout.tv_sec = config->gear_server_rcv_timeout_sec;
    rcv_timeout.tv_usec = config->gear_server_rcv_timeout_usec;
    snd_timeout.tv_sec = config->gear_server_snd_timeout_sec;
    snd_timeout.tv_usec = config->gear_server_snd_timeout_usec;
    synced_stream sync_out;

    while (m_timer.wait_for(std::chrono::microseconds(300))) {
        if (stop_) break;
        int socket = server.accept();
        if (socket > 0) {

            setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, &rcv_timeout, sizeof(rcv_timeout));
            setsockopt(socket, SOL_SOCKET, SO_SNDTIMEO, &snd_timeout, sizeof(snd_timeout));
            client_threadpool.push_task(GearStreamClientHandler::main, socket, messages, databasehandler, databaseTasksThreadpool, &client_threadpool);
#ifdef DEBUG
            //monitor_tasks(&client_threadpool, &sync_out);
#endif
            syslog(LOG_DEBUG, "GearServer %lu/%lu tasks total, %lu tasks running, %zu tasks queued", client_threadpool.get_tasks_total(), client_threadpool.get_thread_count(), client_threadpool.get_tasks_running(), client_threadpool.get_tasks_queued());
        } else if (socket < 0) {
            syslog(LOG_WARNING, "GearServer thread_main got socket < 0, reinit");
            // TODO client cleanup, however should solve itself over time as client connection becomes invalid?
            // TODO sort of error handling?
            initialize();
        }
    }
    syslog(LOG_WARNING, "GearServer thread_main ended");

}

std::thread GearServer::run() {
    return std::thread([this] { this->thread_main(); });
}