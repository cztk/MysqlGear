//
// Created by ztk on 2021-08-08.
//

#include "ThreadpoolHandler.h"

void ThreadpoolHandler::initialize() {
    pool = new thread_pool();
}

void ThreadpoolHandler::initialize(int n) {
    pool = new thread_pool(n);
}

size_t ThreadpoolHandler::get_tasks_queued() const {
    if(nullptr != pool) {
        return pool->get_tasks_queued();
    }
    return 0;
}

size_t ThreadpoolHandler::get_tasks_running() const {
    if(nullptr != pool) {
        return pool->get_tasks_running();
    }
    return 0;
}

size_t ThreadpoolHandler::get_tasks_total() const {
    if(nullptr != pool) {
        return pool->get_tasks_total();
    }
    return 0;
}

size_t ThreadpoolHandler::get_thread_count() const {
    if(nullptr != pool) {
        return pool->get_thread_count();
    }
    return 0;
}

void ThreadpoolHandler::reset(uint32_t nThreads) {
    if(nullptr != pool) {
        return pool->reset(nThreads);
    }
}

thread_pool* ThreadpoolHandler::getPool() {
    return pool;
}

void ThreadpoolHandler::wait_for_tasks() {
    if(nullptr != pool) {
        pool->wait_for_tasks();
    }
}