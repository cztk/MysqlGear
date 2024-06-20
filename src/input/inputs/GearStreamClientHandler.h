//
// Created by ztk on 2021-08-02.
//

#ifndef MYSQLGEAR_GEARSTREAMCLIENTHANDLERTHREAD_H
#define MYSQLGEAR_GEARSTREAMCLIENTHANDLERTHREAD_H


#include <thread>
#include <cstring>
#include <zconf.h>
#include <utility>
#include <sys/socket.h>
#include <condition_variable>
#include <sys/syslog.h>
#include <sstream>
#include "GearInputMessageTypes.h"
#include "../../utils/libzdb/LibzdbHandler.h"
#include "../../utils/timedwaiter/timed_waiter.h"
#include "../../utils/lockedqueue/LockedQueue.h"
#include "../../MysqlGearQueueEntry.h"
#include "../../utils/threadpool/ThreadpoolHandler.h"

#define MAXHEADERSIZE 10
#define BUFSIZE 2048
#define DATABUFSIZE 4096

extern int8_t PROTO_VERSION;

class GearStreamClientHandler {

private:

public:

    static void main(int pSocketFD, utils::LockedQueue<MysqlGearQueueEntry> *pQueue, LibzdbHandler *pDatabasehandler, ThreadpoolHandler *pDatabaseTasksThreadpool, thread_pool *client_threadpool);

    static void addMessage(std::string pData, utils::LockedQueue<MysqlGearQueueEntry> *pQueue);

    static bool sendData(int pSocketFD, const char *message, uint64_t numbytes);
};


#endif //MYSQLGEAR_GEARSTREAMCLIENTHANDLERTHREAD_H
