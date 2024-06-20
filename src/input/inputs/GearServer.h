//
// Created by ztk on 2021-08-02.
//

#ifndef MYSQLGEAR_GEARSERVER_H
#define MYSQLGEAR_GEARSERVER_H


#include <thread>
#include <sys/syslog.h>
#include "GearStreamClientHandler.h"
#include "../../config.h"
#include "../../utils/tcpclientserver/Server.h"
#include "../../utils/timedwaiter/timed_waiter.h"
#include "../../MysqlGearQueueEntry.h"
#include "../../utils/threadpool/thread_pool.hpp"
#include "../../utils/libzdb/LibzdbHandler.h"
#include "../../utils/threadpool/ThreadpoolHandler.h"

class GearServer {

private:
    bool stop_{};
    Config *config;
    utils::LockedQueue<MysqlGearQueueEntry> *messages;
    LibzdbHandler *databasehandler = nullptr;
    ThreadpoolHandler *databaseTasksThreadpool = nullptr;
    timed_waiter m_timer;
    tcp::Server server{};
    void thread_main();

public:
    GearServer(Config *pConfig, utils::LockedQueue<MysqlGearQueueEntry> *pQueue, LibzdbHandler *pDatabasehandler, ThreadpoolHandler *pDatabaseTasksThreadpool);

    ~GearServer();
    void stop();
    std::thread run();
    void initialize();

    static void monitor_tasks(const thread_pool *pool, synced_stream *sync_out);
};


#endif //MYSQLGEAR_GEARSERVER_H
