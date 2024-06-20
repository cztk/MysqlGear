//
// Created by ztk on 2021-08-02.
//

#include <sys/syslog.h>
#include <iostream>
#include "GearStreamClientHandlerThread.h"
#include "GearInputMessageTypes.h"

GearStreamClientHandlerThread::GearStreamClientHandlerThread(int pSocketFD, utils::LockedQueue<MysqlGearQueueEntry> *pQueue) : socket_(pSocketFD), stop_(false),
                                                                                                                               hasInitialData_(false), finished_(false), messages(pQueue) {

}

GearStreamClientHandlerThread::~GearStreamClientHandlerThread() = default;

bool GearStreamClientHandlerThread::sendData(const char *message, uint64_t numbytes) const {
    // TODO error handling
    bool result = false;
    int error_code;
    socklen_t error_code_size = sizeof(error_code);
    getsockopt(socket_, SOL_SOCKET, SO_ERROR, &error_code, &error_code_size);
    if (0 == error_code) {
        auto bytes_sent = ::send(socket_, message, numbytes, 0);
        if (bytes_sent > 0) {
            result = true;
        }
    }
    return result;
}

void GearStreamClientHandlerThread::thread_main() {
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

    //! protocol message type index
    /*!
        contains the current message type number we are trying to process.
        Use only if has_message_type_ is set to true.
    */
    int8_t message_type_ = 15;

    std::string payload_data;
    uint32_t payload_data_size = 0;
    uint32_t payload_data_read = 0;
    bool answered = false;

    while (!stop_) {
        bzero(buffer, BUFSIZE);
        numbytes = recv(socket_, buffer, BUFSIZE - 1, 0);
        numbytes += leftoversize;

        if (0 < numbytes) {
            memcpy(data + leftoversize, buffer, BUFSIZE);

            //n = readmessages(data, leftover, numbytes + n);

            {
                pos = 0;

                while (pos < numbytes && !stop_) {

                    if (!has_message_type_) {
                        message_type_ = data[pos];
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
                                break;
                            }
                        }

                        switch (message_type_) {
                            case N_CONNECT:
                            case N_STATUS:
                            case N_CONTROL:
                            default:
                                break;
                            case N_MYSQLDATA:
                                if(payload_data_read < payload_data_size) {
                                    while(pos < numbytes && payload_data_read < payload_data_size) {
                                        payload_data += data[pos];
                                        pos++;
                                        payload_data_read++;
                                    }
                                }
                                if(payload_data_read == payload_data_size) {
                                    addMessage(payload_data);
                                    const char rdata = '1';
                                    auto written = write(socket_, &rdata, 1);
                                    answered = true;
                                    has_message_type_ = false;
                                    has_message_header_ = false;
                                    payload_data = "";
                                    payload_data_read = 0;
                                    payload_data_size = 0;
                                }
                                break;
                        }
                    }
                }
            }
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
        const char rdata = '0';
        auto written = write(socket_, &rdata, 1);
    }

    close(socket_);
    socket_ = 0;
    finished_ = true;
}

void GearStreamClientHandlerThread::stop() {
    {
        std::lock_guard<std::mutex> l(m_);
        stop_ = true;
    }
    twaiter.interrupt();
}

bool GearStreamClientHandlerThread::isStopped() const {
    return stop_;
}

std::thread GearStreamClientHandlerThread::run() {
    return std::thread([this] { this->thread_main(); });
}

void GearStreamClientHandlerThread::addMessage(std::string pData) {
    MysqlGearQueueEntry entry{};
    entry.query = std::move(pData);
    entry.received = std::chrono::duration_cast<std::chrono::seconds>
            (std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    messages->push(entry);
}

bool GearStreamClientHandlerThread::hasInitialData() const {
    return hasInitialData_;
}

void GearStreamClientHandlerThread::setHasInitialData(bool b) {
    hasInitialData_ = b;
}
