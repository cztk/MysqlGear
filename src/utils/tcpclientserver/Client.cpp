//
// Created by zysik on 17.11.20.
//

#include "Client.h"

namespace tcp {

    Client::Client(const char *host, uint16_t port) : host_(host), port_(port), socket_desc_(0),
                                                      is_connected_(false) {

    }

    Client::~Client() {
        shutdown();
    }

    bool Client::connect() {
        shutdown();
        socket_desc_ = socket(AF_INET, SOCK_STREAM, 0);
        //printf("socket %i, ", socket_desc_);
        if (socket_desc_ != -1) {
            int enable = 1;
            int setr = setsockopt(socket_desc_, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
#ifdef DEBUG
            printf("setsocketopt SO_REUSEADDR result: %i, ", setr);
#endif
            struct KeepConfig cfg = {30, 5, 5};
            int kar = set_tcp_keepalive_cfg(socket_desc_, &cfg);
#ifdef DEBUG
            printf("set keepalive result: %i, ", kar);
#endif
            struct sockaddr_in server{};
            server.sin_addr.s_addr = inet_addr(host_);
            server.sin_family = AF_INET;
            server.sin_port = htons(port_);
            int conr = ::connect(socket_desc_, (struct sockaddr *) &server, sizeof(server));
            //printf("conn_r %i", conr);
            is_connected_ = (conr == 0);
            //printf("\n");
        }

        return is_connected_;
    }

    bool Client::isConnected() const {
        return is_connected_;
    }

    void Client::shutdown() {
        if (socket_desc_ > 0) {
            ::shutdown(socket_desc_, SHUT_RDWR);
            close(socket_desc_);
        }
        socket_desc_ = -1;
        is_connected_ = false;
    }

    ssize_t Client::recv(void *buffer, size_t size) {
        ssize_t result = -1;
        if (is_connected_) {
            result = ::recv(socket_desc_, buffer, size, 0);
            if (result < 0) {
                is_connected_ = false;
            }
        }
        return result;
    }

    ssize_t Client::send(const char *message, uint64_t numbytes) {
        // TODO error handling
        ssize_t result = -1;
        if (is_connected_) {
            int error_code;
            socklen_t error_code_size = sizeof(error_code);
            getsockopt(socket_desc_, SOL_SOCKET, SO_ERROR, &error_code, &error_code_size);
            if (0 != error_code) {
                result = error_code;
                is_connected_ = false;
            } else {
                result = ::send(socket_desc_, message, numbytes, 0);
                if (result < 0) {
                    is_connected_ = false;
                }
            }
        }
        return result;
    }

    int Client::set_tcp_keepalive(int sockfd) {
        int optval = 1;

        return setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval));
    }

    int Client::set_tcp_keepalive_cfg(int sockfd, const struct KeepConfig *cfg) {
        int rc;

        //first turn on keepalive
        rc = set_tcp_keepalive(sockfd);
        if (rc != 0) {
            return rc;
        }

        //set the keepalive options
        rc = setsockopt(sockfd, IPPROTO_TCP, TCP_KEEPCNT, &cfg->keepcnt, sizeof cfg->keepcnt);
        if (rc != 0) {
            return rc;
        }

        rc = setsockopt(sockfd, IPPROTO_TCP, TCP_KEEPIDLE, &cfg->keepidle, sizeof cfg->keepidle);
        if (rc != 0) {
            return rc;
        }

        rc = setsockopt(sockfd, IPPROTO_TCP, TCP_KEEPINTVL, &cfg->keepintvl, sizeof cfg->keepintvl);
        if (rc != 0) {
            return rc;
        }

        return 0;
    }

    void Client::getMacAddress(const char *dev, unsigned char *dest) {
        int fd;

        struct ifreq ifr{};
        char *mac;

        fd = socket(AF_INET, SOCK_DGRAM, 0);
        ifr.ifr_addr.sa_family = AF_INET;
        strncpy((char *) ifr.ifr_name, dev, IFNAMSIZ - 1);
        ioctl(fd, SIOCGIFHWADDR, &ifr);
        close(fd);

        mac = (char *) ifr.ifr_hwaddr.sa_data;
        memcpy(dest, mac, 6);
    }
}