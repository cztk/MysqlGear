//
// Created by ztk on 2021-08-02.
//

#include "OutputHandler.h"


OutputHandler::OutputHandler(Config *pConfig, utils::LockedQueue<MysqlGearQueueEntry> *pQueue,
                             LibzdbHandler *pDatabaseHandler, ThreadpoolHandler *pThreadpoolHandler) : config(pConfig),
                                                                                                       messages(pQueue),
                                                                                                       stop_(false),
                                                                                                       databasehandler(
                                                                                                               pDatabaseHandler),
                                                                                                       pool(pThreadpoolHandler) {
}

OutputHandler::~OutputHandler() = default;

void OutputHandler::stop() {
    {
        stop_ = true;
    }
}

std::thread OutputHandler::run() {
    return std::thread([this] { this->thread_main(); });
}


void OutputHandler::task(LibzdbHandler *pDatabaseHandler, MysqlGearQueueEntry message,
                         utils::LockedQueue<MysqlGearQueueEntry> *pmessages) {
    //std::unique_lock<std::mutex> l(conpoolmutex);
    uint64_t now = std::chrono::duration_cast<std::chrono::seconds>
            (std::chrono::high_resolution_clock::now().time_since_epoch()).count();

    if (nullptr == pDatabaseHandler) {
#ifdef DEBUG
        std::cout << "connection pool is not initialized, rescheduling\n";
        std::cout << message.query << "\n";
#endif
        syslog(LOG_WARNING, "connection pool is not initialized, rescheduling %s (%i)",
               message.query.c_str(), message.attempts);
        pmessages->push(message);
        return;
    }

    if (pDatabaseHandler->hasConnectionPool()) {

        try {

            try {
                auto con = pDatabaseHandler->getConnection();

                if (nullptr == con) {
#ifdef DEBUG
                    std::cout << "got null con, rescheduling\n";
                    std::cout << message.query << "\n";
#endif
                    syslog(LOG_WARNING, "could not get connection from pool, rescheduling %s (%i)",
                           message.query.c_str(), message.attempts);

                    pmessages->push(message);
                    return;
                }

                try {
                    auto result = con.executeQuery(message.query.c_str());
                } catch (zdb::sql_exception &e) {
                    std::string exceptionMessage = e.what();
#ifdef DEBUG
                    std::cout << exceptionMessage << "\n";
                    std::cout << message.query << "\n";
#endif
                    syslog(LOG_ERR, "error: %s ,query: %s", exceptionMessage.c_str(), message.query.c_str());
                    if (exceptionMessage.find("Deadlock found") != std::string::npos) {
                        syslog(LOG_ERR, "rescheduling %s (%i)", message.query.c_str(), message.attempts);
                        pmessages->push(message);
                    } else {
                        message.attempts++;
                    }


                    //TODO check for non syntax errors like deadlocks and reschedule if applicable
                }
                pDatabaseHandler->returnConnection(&con);
            } catch (LibzdbHandlerNoConPoolException &e) {
#ifdef DEBUG
                std::cout << "Outputhandler got no connection " << e.what() << "\n";
#endif
            }
            return;
        } catch (zdb::sql_exception &e) {
#ifdef DEBUG
            std::cout << e.what() << "\n";
            std::cout << message.query << "\n";
            std::cout << "con error, rescheduling\n";
            std::cout << message.query << "\n";
#endif
            syslog(LOG_ALERT, "error: %s ,query: %s ,rescheduling (%i)", e.what(),
                   message.query.c_str(), message.attempts);
            pmessages->push(message);
        }
    } else {
#ifdef DEBUG
        std::cout << message.query << "\n";
        std::cout << "con error ( no connection pool ), rescheduling\n";
#endif
        syslog(LOG_ALERT, "error: no connection pool ,query: %s ,rescheduling (%i)",
               message.query.c_str(), message.attempts);
    }
}

void OutputHandler::thread_main() {
    MysqlGearQueueEntry message;
    setupPool();

    while (m_timer.wait_for(std::chrono::microseconds(20))) {
        if (stop_) break;
        try {
            if (nullptr == databasehandler || !databasehandler->hasConnectionPool()) {
#ifdef DEBUG
                std::cout << "OutputHandler  databasehandler == nullptr -> setupPool" << "\n";
#endif
                setupPool();
            } else {
                if (!messages->empty()) {
                    if (nullptr != pool) {
                        auto _pool = pool->getPool();
                        if (nullptr != _pool) {
                            while (!messages->empty()) {
                                if (pool->get_tasks_running() < databasehandler->getMaxDatabaseConnections()) {
                                    message = messages->pop();

                                    if (!config->obey_work_retry_count || message.attempts < config->work_retry_count) {
                                        _pool->push_task(OutputHandler::task, databasehandler, message, messages);
                                    }
                                } else {
#ifdef DEBUG
                                    std::cout << "OutputHandler pool tasks running" << pool->get_tasks_running()
                                              << " < ";
                                    std::cout << databasehandler->getMaxDatabaseConnections() << "  != true\n";
#endif
                                }
                            }
                        } else {
#ifdef DEBUG
                            std::cout << "OutputHandler _pool is null" << "\n";
#endif
                        }
                    }
                }
            }
        }
        catch (const std::exception &e) {
#ifdef DEBUG
            std::cout << "Outputhandler thread_main fatal: " << e.what() << "\n";
#endif
        }
    }

    deinitialize();
}

void OutputHandler::transmit(const std::string &msg) {

}


void OutputHandler::deinitialize() {
    if (nullptr != pool) {
        pool->wait_for_tasks();
    }

    if (databasehandler != nullptr) {
        databasehandler->disconnect();
    }
}

void OutputHandler::setupPool() {
    std::unique_lock<std::mutex> l(conpoolmutex);
    if (nullptr != pool) {
        pool->wait_for_tasks();
    }
    if (databasehandler != nullptr) {
        databasehandler->connect();
    }
}