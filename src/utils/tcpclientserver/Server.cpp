//
// Created by zysik on 17.11.20.
//

#include "Server.h"

namespace tcp {


    Server::Server() : addr_size(sizeof serverStorage) {

    }

    Server::~Server() = default;

    int Server::accept() {
        // TODO save some sort of last_error not only socket errors
        if (nullptr == pfds) {
            return -1;
        }
        int poll_count = poll(pfds, 1, 600);

        if (poll_count == -1) {
            return -1;
        }

        // TODO some general purpose net functions
        int error_code;
        socklen_t error_code_size = sizeof(error_code);
        getsockopt(serverSocket, SOL_SOCKET, SO_ERROR, &error_code, &error_code_size);
        if (0 != error_code) {
            return -1;
        }

        if (pfds[0].revents & POLLIN) { // We got one!!
            if (pfds[0].fd == serverSocket) { // make sure its our server socket
                auto new_socket = ::accept(serverSocket, (struct sockaddr *) &serverStorage, &addr_size);
                if (new_socket == -1) {
                    // TODO what are possible error_codes if and only if client con aborted just in between
                    return 0;
                }
                return new_socket;
            }
        }

        return 0;
    }

    void Server::shutdown() {
        if (serverSocket > 0) {
            ::shutdown(serverSocket, SHUT_RDWR);
            close(serverSocket);
        }
        if (nullptr != pfds) {
            free(pfds);
            pfds = nullptr;
        }
        serverSocket = -1;
    }

    int
    Server::setup(const char *interface, const char *host, uint16_t port, bool bind_to_interface = false,
                  int backlog = 50) {
        // make sure we do not leave any socket open
        shutdown();

        //Create the socket.
        serverSocket = socket(AF_INET, SOCK_STREAM, 0);

        // enable socket reuse
        int enable = 1;
        if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
            shutdown();
            return 1;
        }

        // bind to interface
        if (bind_to_interface) {
            struct ifreq ifr{};
            memset(&ifr, 0, sizeof(ifr));
            snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "%s", interface);
            if (setsockopt(serverSocket, SOL_SOCKET, SO_BINDTODEVICE, (void *) &ifr, sizeof(ifr)) < 0) {
                // failed to use interface
                shutdown();
                return 1;
            }
        }

        // Configure settings of the server address struct
        // Address family = Internet
        serverAddr.sin_family = AF_INET;

        //Set port number, using htons function to use proper byte order
        serverAddr.sin_port = htons(port);

        //Set IP address to localhost
        serverAddr.sin_addr.s_addr = inet_addr(host);

        //Set all bits of the padding field to 0
        memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

        //Bind the address struct to the socket
        int bind_result = bind(serverSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));
        if (0 == bind_result) {
            //Listen on the socket, with (backlog) max connection requests queued
            if (listen(serverSocket, backlog) == 0) {
                // Add the listener to set
                if (nullptr != pfds) {
                    free(pfds);
                    pfds = nullptr;
                }
                pfds = static_cast<pollfd *>(malloc(sizeof *pfds));
                pfds[0].fd = serverSocket;
                pfds[0].events = POLLIN; // Report ready to read on incoming connection

                return 0;
            } else {
                shutdown();
                return 1;
            }
        } else {
            shutdown();
            return 1;
        }


    }
}