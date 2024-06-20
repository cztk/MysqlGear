//
// Created by ztk on 2021-07-17.
//

#ifndef MYSQLGEAR_INPUTHANDLER_H
#define MYSQLGEAR_INPUTHANDLER_H

#include <thread>
#include "../config.h"
#include "../utils/timedwaiter/timed_waiter.h"
#include "../utils/lockedqueue/LockedQueue.h"
#include "inputs/GearServer.h"
#include "../MysqlGearQueueEntry.h"
#include "../utils/libzdb/LibzdbHandler.h"
#include "../utils/threadpool/ThreadpoolHandler.h"

class InputHandler {
private:

    Config *config;
    utils::LockedQueue<MysqlGearQueueEntry> *messages;
    LibzdbHandler *databasehandler = nullptr;
    ThreadpoolHandler *databaseTasksThreadpool = nullptr;

    GearServer *gearServer = nullptr;
    std::thread gearServerThread;
public:
    //! constructor.
    /*!

    */
    InputHandler(Config *config, utils::LockedQueue<MysqlGearQueueEntry> *pQueue, LibzdbHandler *pDatabaseHandler, ThreadpoolHandler *pDatabaseTasksThreadpool);

    //! destructor.
    /*!
    */
    ~InputHandler();

    void initialize();
    void deinitialize();
};


#endif //MYSQLGEAR_INPUTHANDLER_H
