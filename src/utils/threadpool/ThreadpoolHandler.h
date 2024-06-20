//
// Created by ztk on 2021-08-08.
//

#ifndef MYSQLGEAR_THREADPOOLHANDLER_H
#define MYSQLGEAR_THREADPOOLHANDLER_H


#include "thread_pool.hpp"

class ThreadpoolHandler {
private:
    thread_pool *pool = nullptr;
public:

    void initialize();

    size_t get_tasks_queued() const;

    size_t get_tasks_running() const;

    size_t get_tasks_total() const;

    size_t get_thread_count() const;

    void reset(uint32_t nThreads);

    void wait_for_tasks();

    thread_pool *getPool();

    void initialize(int n);
};


#endif //MYSQLGEAR_THREADPOOLHANDLER_H
