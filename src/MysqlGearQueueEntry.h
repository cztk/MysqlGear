//
// Created by ztk on 2021-08-03.
//

#ifndef MYSQLGEAR_MYSQLGEARQUEUEENTRY_H
#define MYSQLGEAR_MYSQLGEARQUEUEENTRY_H

struct MysqlGearQueueEntry {
    std::string query = "";
    std::string lasterror = "";
    uint64_t received = 0;
    uint64_t processed = 0;
    int attempts = 0;
};

#endif //MYSQLGEAR_MYSQLGEARQUEUEENTRY_H
