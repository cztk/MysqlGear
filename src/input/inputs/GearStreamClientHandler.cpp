//
// Created by ztk on 2021-08-02.
//

#include "GearStreamClientHandler.h"


bool GearStreamClientHandler::sendData(int pSocket, const char *message, uint64_t numbytes) {
    // TODO error handling
    bool result = false;
    int error_code;
    socklen_t error_code_size = sizeof(error_code);
    getsockopt(pSocket, SOL_SOCKET, SO_ERROR, &error_code, &error_code_size);
    if (0 == error_code) {
        auto bytes_sent = ::send(pSocket, message, numbytes, 0);
        if (bytes_sent > 0) {
            result = true;
        }
    }
    return result;
}

void GearStreamClientHandler::main(int pSocketFD, utils::LockedQueue<MysqlGearQueueEntry> *pQueue, LibzdbHandler *pDatabasehandler, ThreadpoolHandler *pDatabaseTasksThreadpool, thread_pool *client_threadpool) {
    ssize_t numbytes;
    ssize_t pos;
    size_t leftoversize = 0;

    char buffer[BUFSIZE];
    char data[DATABUFSIZE];
    unsigned char header_message[MAXHEADERSIZE]; // should fit largest header

    bzero(data, DATABUFSIZE);
    bzero(buffer, BUFSIZE);

    //! protocol message type available flag
    /*!
        if we are unaware of the current message type of processed data we treat the next
        data as message type and set this to true if the message type ( which is a numerical value )
        is not exceeding our max index of available types.
    */
    bool has_message_type_ = false;
    bool has_message_header_ = false;
    bool has_message_proto_ = false;

    //! protocol message type index
    /*!
        contains the current message type number we are trying to process.
        Use only if has_message_type_ is set to true.
    */
    int8_t message_type_ = 15;
    int8_t message_proto = 0;

    std::string payload_data;
    uint32_t payload_data_size = 0;
    uint32_t payload_data_read = 0;
    bool answered = false;
    bool stop_ = false;
    while (!stop_) {
        bzero(buffer, BUFSIZE);
        numbytes = recv(pSocketFD, buffer, BUFSIZE - 1, 0);
        numbytes += leftoversize;

        if (0 < numbytes) {
            memcpy(data + leftoversize, buffer, BUFSIZE);

            //n = readmessages(data, leftover, numbytes + n);

            {
                pos = 0;

                while (pos < numbytes && !stop_) {

                    if(!has_message_proto_) {
                        char sendDataStr = '0';
                        message_proto = data[pos++];
                        if(message_proto < 1 || message_proto > PROTO_VERSION) {
                            auto written = GearStreamClientHandler::sendData(pSocketFD, &sendDataStr, 1);
#ifdef DEBUG
                            std::cout << "client sent proto " << (int)message_proto << " which we do not support max proto is " << PROTO_VERSION << "\n";
#endif
                            answered = true;
                            stop_ = true;
                            break;
                        } else {
                            sendDataStr = '1';
                            has_message_proto_ = true;
                            auto written = GearStreamClientHandler::sendData(pSocketFD, &sendDataStr, 1);
#ifdef DEBUG
                            std::cout << "client sent proto " << (int)message_proto << " which we can work with\n";
#endif
                        }
                    } else {
                        if (!has_message_type_) {
                            message_type_ = data[pos];
#ifdef DEBUG
                            std::cout << "client sent message_type " << (int)message_type_ << "\n";
#endif
                            if (message_type_ < NETMESSAGES::N_NUMMSG) {
                                has_message_type_ = true;
                            } else {
                                syslog(LOG_ALERT,
                                       ": ClientHandler received unknown message type (%i) from client",
                                       message_type_);
                            }
                            pos++;
                        } else {
                            if(!has_message_header_) {
                                auto header_len = netmessages_header_size[message_type_];

                                if (header_len + pos <= numbytes) {
                                    has_message_header_ = true;
                                    bzero(header_message, MAXHEADERSIZE);
                                    if (header_len + pos <= DATABUFSIZE) {
                                        memcpy(&header_message, data + pos, header_len);
                                    }
                                    payload_data_size = 0;
                                    if(header_len >= 4) {
                                        payload_data_size = header_message[0] | (header_message[1] << 8) | (header_message[2] << 16) | (header_message[3] << 24);
                                    }
                                    payload_data_read = 0;
                                    pos += header_len;
                                } else {
#ifdef DEBUG
                                    std::cout << "not enough data to parse header " << numbytes << "\n";
#endif
                                    break;
                                }
                            }

                            switch (message_type_) {
                                case N_CONNECT:
                                    case N_CONTROL:
                                        default:
                                            break;
                                case N_STATUS:
                                {
                                    std::stringstream sendDataStream;

                                    if(nullptr != pQueue) {
                                        sendDataStream << pQueue->size() << ";";
                                    } else {
                                        sendDataStream << 0 << ";";
                                    }

                                    if(nullptr != pDatabasehandler) {
                                        sendDataStream << pDatabasehandler->getMaxDatabaseConnections() << ";";
                                        sendDataStream << pDatabasehandler->getActiveDatabaseConnections() << ";";
                                    } else {
                                        sendDataStream << "0" << ";";
                                        sendDataStream << "0" << ";";
                                    }

                                    if(nullptr != client_threadpool) {
                                        sendDataStream << client_threadpool->get_thread_count() << ";";
                                        sendDataStream << client_threadpool->get_tasks_running() << ";";
                                        sendDataStream << client_threadpool->get_tasks_queued() << ";";
                                        sendDataStream << client_threadpool->get_tasks_total() << ";";
                                    } else {
                                        sendDataStream << "0" << ";";
                                        sendDataStream << "0" << ";";
                                        sendDataStream << "0" << ";";
                                        sendDataStream << "0" << ";";
                                    }

                                    if(nullptr != pDatabaseTasksThreadpool) {
                                        sendDataStream << pDatabaseTasksThreadpool->get_thread_count() << ";";
                                        sendDataStream << pDatabaseTasksThreadpool->get_tasks_running() << ";";
                                        sendDataStream << pDatabaseTasksThreadpool->get_tasks_queued() << ";";
                                        sendDataStream << pDatabaseTasksThreadpool->get_tasks_total();
                                    } else {
                                        sendDataStream << "0" << ";";
                                        sendDataStream << "0" << ";";
                                        sendDataStream << "0" << ";";
                                        sendDataStream << "0";
                                    }

                                    std::string sendDataStr;
                                    char statusVarsLength = sendDataStream.str().length(); // if we exceed int amount of data, we should worry for other things :D
                                    sendDataStr += statusVarsLength;
                                    sendDataStr += sendDataStream.str();
#ifdef DEBUG
                                    std::cout << "sent " << sendDataStr << "\n";
#endif
                                    auto written = GearStreamClientHandler::sendData(pSocketFD, sendDataStr.c_str(), sendDataStr.length());
                                    answered = true;
                                    has_message_type_ = false;
                                    has_message_header_ = false;
                                    payload_data = "";
                                    payload_data_read = 0;
                                    payload_data_size = 0;
                                    break;
                                }
                                case N_MYSQLDATA:
                                {
                                    if(payload_data_read < payload_data_size) {
                                        while(pos < numbytes && payload_data_read < payload_data_size) {
                                            payload_data += data[pos];
                                            pos++;
                                            payload_data_read++;
                                        }
                                    }
                                    if(payload_data_read == payload_data_size) {
#ifdef DEBUG
                                        std::cout << "adding query " << payload_data << "\n";
#endif
                                        GearStreamClientHandler::addMessage(payload_data, pQueue);
                                        auto written = GearStreamClientHandler::sendData(pSocketFD, "1", 1);
                                        answered = true;
                                        has_message_type_ = false;
                                        has_message_header_ = false;
                                        payload_data = "";
                                        payload_data_read = 0;
                                        payload_data_size = 0;
                                    }
                                    break;
                                }
                            } // end switch cashe
                        } // end has type
                    } // end has proto
                } // end while
            }
#ifdef DEBUG
std::cout << "numbytes " << numbytes << "\n";
            std::cout << "pos " << pos << "\n";
#endif
            leftoversize = numbytes - pos;
            if (leftoversize <= DATABUFSIZE) {
                memcpy(data, &data[pos], leftoversize);
            }

        } else {
            stop_ = true;
            break;
        }
    }
    if(!answered) {
#ifdef DEBUG
        std::cout << "sending last resort answer 0 "<< "\n";
#endif
        auto written = GearStreamClientHandler::sendData(pSocketFD, "0", 1);
    }
#ifdef DEBUG
    std::cout << "we are done "<< "\n";
#endif
    close(pSocketFD);
}

void GearStreamClientHandler::addMessage(std::string pData, utils::LockedQueue<MysqlGearQueueEntry> *pQueue) {
    MysqlGearQueueEntry entry{};
    entry.query = std::move(pData);
    entry.received = std::chrono::duration_cast<std::chrono::seconds>
            (std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    pQueue->push(entry);
}