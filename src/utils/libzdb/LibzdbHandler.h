//
// Created by ztk on 2021-08-07.
//

#ifndef MYSQLGEAR_LIBZDBHANDLER_H
#define MYSQLGEAR_LIBZDBHANDLER_H

#include <memory>
#include <unistd.h>
#include <zdb/zdbpp.h>
#include <sys/syslog.h>
#include <chrono>
#include <optional>
#include <functional>
#ifdef DEBUG
#include <iostream>
#endif
#include "../../config.h"

struct LibzdbHandlerNoConPoolException : public std::exception {
    [[nodiscard]] const char * what () const noexcept override {
        return "no connection pool exception";
    }
};

class LibzdbHandler {
private:
    Config *config;
    size_t dbserverindex;

    zdb::ConnectionPool *conpool = nullptr;
    static void zdbErrorHandler(const char *error);
    void setConnectionFaulty(size_t index);
    bool setViableConnectionConfigIndex();
public:

    explicit LibzdbHandler(Config *pConfig);


    int getActiveDatabaseConnections();
    int getMaxDatabaseConnections();


    void connect();
    void disconnect();

    bool ping();

    bool setMaxDatabaseConnections(int n);

    zdb::Connection getConnection();
    void returnConnection(zdb::Connection *con);

    bool hasConnectionPool();
};


#endif //MYSQLGEAR_LIBZDBHANDLER_H
