//
// Created by ztk on 2021-08-02.
//

#ifndef MYSQLGEAR_OUTPUTHANDLER_H
#define MYSQLGEAR_OUTPUTHANDLER_H

#include <thread>
#include <zdb/zdbpp.h>
#include <sys/syslog.h>
#include <chrono>
#ifdef DEBUG
#include <iostream>
#endif
#include <unistd.h>
#include "../config.h"
#include "../utils/timedwaiter/timed_waiter.h"
#include "../utils/lockedqueue/LockedQueue.h"
#include "../MysqlGearQueueEntry.h"
#include "../utils/libzdb/LibzdbHandler.h"
#include "../utils/threadpool/ThreadpoolHandler.h"

class OutputHandler {
private:

    Config *config;
    utils::LockedQueue<MysqlGearQueueEntry> *messages;

    bool stop_;
    LibzdbHandler *databasehandler = nullptr;

    timed_waiter m_timer;
    std::mutex conpoolmutex;


    ThreadpoolHandler *pool;

    void thread_main();
    void deinitialize();
    void setupPool();
    void transmit(const std::string& string);
public:
    //! constructor.
    /*!

    */
    OutputHandler(Config *pConfig, utils::LockedQueue<MysqlGearQueueEntry> *pQueue, LibzdbHandler *pDatabaseHandler, ThreadpoolHandler *pThreadpoolHandler);

    //! destructor.
    /*!
    */
    ~OutputHandler();
    void stop();
    std::thread run();

    static void task(LibzdbHandler *pDatabaseHandler, MysqlGearQueueEntry message, utils::LockedQueue<MysqlGearQueueEntry> *pmessages);
};


#endif //MYSQLGEAR_OUTPUTHANDLER_H
