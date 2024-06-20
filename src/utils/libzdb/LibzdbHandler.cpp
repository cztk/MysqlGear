//
// Created by ztk on 2021-08-07.
//

#include "LibzdbHandler.h"

void LibzdbHandler::zdbErrorHandler(const char *error) {
#ifdef DEBUG
    std::cout << "zdbErrorHandler " << "\n";
    std::cout << error << "\n";
#endif
    syslog(LOG_ERR, "zdb Error %s", error);
}

int LibzdbHandler::getActiveDatabaseConnections() {
    if(nullptr != conpool)
        return conpool->active();
    else
        return 0;
}

int LibzdbHandler::getMaxDatabaseConnections() {
    if(nullptr != conpool)
        return conpool->getMaxConnections();
    else
        return 0;
}

bool LibzdbHandler::setMaxDatabaseConnections(int n) {
    if(nullptr != conpool) {
        if( n > conpool->getInitialConnections() ) {
            conpool->setMaxConnections(n);
            return true;
        }
    }
    return false;
}

bool LibzdbHandler::ping() {
    bool pingr = false;
    if (conpool != nullptr) {
        try {
            auto con = conpool->getConnection();
            pingr = con.ping();
            conpool->returnConnection(con);
        } catch (...) {
        }
    }
    return pingr;
}

void LibzdbHandler::setConnectionFaulty(size_t index) {
    if (0 >= index && index < this->config->mysql_server.size()) {
#ifdef DEBUG
        std::cout << "setting connection faulty\n";
#endif
        this->config->mysql_server[index].last_error = std::chrono::duration_cast<std::chrono::seconds>
                (std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    }
}

bool LibzdbHandler::setViableConnectionConfigIndex() {
    uint64_t now = std::chrono::duration_cast<std::chrono::seconds>
            (std::chrono::high_resolution_clock::now().time_since_epoch()).count();

    for (size_t i = 0; i != this->config->mysql_server.size(); i++) {
        if (0 == this->config->mysql_server[i].last_error || (30 < (now - this->config->mysql_server[i].last_error))) {
            dbserverindex = i;
            return true;
        }
    }
    return false;
}

zdb::Connection LibzdbHandler::getConnection() {
    if (conpool != nullptr) {
        try {
            return conpool->getConnection();
        } catch (...) {
        }
    }
    throw LibzdbHandlerNoConPoolException();
}

void LibzdbHandler::returnConnection(zdb::Connection *con) {
    if (conpool != nullptr) {
        try {
            conpool->returnConnection(*con);
        } catch (...) {
        }
    }
}

void LibzdbHandler::disconnect() {
    if (conpool != nullptr) {
        try {
            syslog(LOG_INFO, "calling stop on conpool");
            conpool->stop();
            syslog(LOG_INFO, "deleting conpool");
            delete conpool;
        } catch (...) {
        }
        conpool = nullptr;
    }
}

void LibzdbHandler::connect() {
    if(nullptr == config) return;
    bool pingr = false;

    pingr = ping();
    if (pingr) {
        return;
    } else {
        if (conpool != nullptr) {
            setConnectionFaulty(dbserverindex);
        }
        disconnect();
    }

    try {

        if(setViableConnectionConfigIndex()) {
            if (dbserverindex < this->config->mysql_server.size()) {
                conpool = new zdb::ConnectionPool(config->mysql_server[dbserverindex].connection_string.c_str());
                conpool->setAbortHandler(this->zdbErrorHandler);
                //TODO setting per connection string
                conpool->setInitialConnections(config->mysql_initialconnections);
                conpool->setMaxConnections(config->mysql_maxconnections);
                conpool->start();

                pingr = ping();

#ifdef DEBUG
                std::cout << "conpool started using " << config->mysql_server[dbserverindex].connection_string << "\n";
#endif
                syslog(LOG_INFO, "onpool started using: %s", config->mysql_server[dbserverindex].connection_string.c_str());
            } else {
#ifdef DEBUG
                std::cout << "got dbserverindex: '" << dbserverindex << "'\n";
#endif
            }
        } else {
            syslog(LOG_INFO, "got no viable database connection config");
#ifdef DEBUG
            std::cout << "got no viable database connection config" << "\n";
#endif

            usleep(10000000);
        }
    }
    catch (zdb::sql_exception &e) {
        if (dbserverindex < this->config->mysql_server.size()) {
            config->mysql_server[dbserverindex].last_error = std::chrono::duration_cast<std::chrono::seconds>
                    (std::chrono::high_resolution_clock::now().time_since_epoch()).count();
        }
#ifdef DEBUG
        std::cout << "connect fatal error: " << e.what() << "\n";
#endif
        syslog(LOG_ALERT, "error: %s", e.what());
    }
}

LibzdbHandler::LibzdbHandler(Config *pConfig) : config(pConfig) {

}

bool LibzdbHandler::hasConnectionPool() {
    return (conpool != nullptr);
}
