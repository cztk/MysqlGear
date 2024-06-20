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
#include "../../utils/timedwaiter/timed_waiter.h"
#include "../../utils/lockedqueue/LockedQueue.h"
#include "../../MysqlGearQueueEntry.h"

#define MAXHEADERSIZE 10
#define BUFSIZE 2048
#define DATABUFSIZE 4096

class GearStreamClientHandlerThread {

private:
    int socket_;
    bool stop_;
    bool hasInitialData_;
    bool finished_;
    utils::LockedQueue<MysqlGearQueueEntry> *messages;
    std::mutex m_;
    timed_waiter twaiter;


    void thread_main();


public:
    GearStreamClientHandlerThread(int pSocketFD, utils::LockedQueue<MysqlGearQueueEntry> *pQueue);

    ~GearStreamClientHandlerThread();

    std::thread run();

    void stop();

    [[nodiscard]] bool isStopped() const;

    void addMessage(std::string pData);

    [[nodiscard]] bool hasInitialData() const;

    void setHasInitialData(bool b);

    bool sendData(const char *message, uint64_t numbytes) const;
};


#endif //MYSQLGEAR_GEARSTREAMCLIENTHANDLERTHREAD_H
