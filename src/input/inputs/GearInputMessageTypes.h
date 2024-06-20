//
// Created by ztk on 2021-08-02.
//

#ifndef MYSQLGEAR_GEARINPUTMESSAGETYPES_H
#define MYSQLGEAR_GEARINPUTMESSAGETYPES_H
// enumerated list of available message types
enum NETMESSAGES : uint8_t {
    N_CONNECT = 0, N_STATUS, N_CONTROL, N_MYSQLDATA,
    N_NUMMSG
};

// enumerated list of header size for each message type
static const ssize_t netmessages_header_size[] = {
        10, // N_CONNECT 6 bytes client id (mac) not implemented / not needed
        4, // N_STATUS generic request, //TODO specify status of specified server not implemented / not really needed yet
        4, // N_CONTROL //TODO think of general control message with some payload or individual message types to uhm control stuff
        4, // N_MYSQLDATA set a bit to 1 or 0 to send query to primary master if ever using some master/master setup
        0
};
#endif //MYSQLGEAR_GEARINPUTMESSAGETYPES_H
