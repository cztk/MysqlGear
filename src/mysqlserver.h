//
// Created by ztk on 2021-07-17.
//

#ifndef MYSQLGEAR_MYSQLSERVER_H
#define MYSQLGEAR_MYSQLSERVER_H

struct MysqlServer {
    std::string connection_string;
    long last_error = 0;
};

#endif //MYSQLGEAR_MYSQLSERVER_H
