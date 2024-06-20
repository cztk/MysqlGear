//
// Created by zysik on 17.11.20.
//

#ifndef TCP_SERVER_H
#define TCP_SERVER_H

/// @cond
#include <cstdio>
#include <atomic>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <net/if.h>
#include <zconf.h>
#include <poll.h>
#include <cstdlib>

/// @endcond


namespace tcp {
//!  handle basic TCP server socket tasks
/*!
  simple tcp networking server class for linux based systems to handle
  client connections.
*/
    class Server {
    private:
        //! socket descriptor
        int serverSocket = -1;

        //! settings of the server socket
        struct sockaddr_in serverAddr{};

        //! settings of the server socket
        struct sockaddr_storage serverStorage{};

        //! length of ADDR
        socklen_t addr_size;

        pollfd *pfds = nullptr;

    public:
        //! default constructor
        /*!
            sets addr_size to sizeof serverStorage
        */
        Server();

        //! default destructor
        /*!
        */
        ~Server();

        void shutdown();

        //! setup server socket
        /*!
            attempt to create a socket,
            enable SO_REUSEADDR to reuse the ip/port combination after a restart without having to wait for too long
            if bind_to_interface is set to true ( default is false ), attempt to bind to given interface
            binding to interface requires root privileges, if it fails this function returns (1).


        */
        int setup(const char *interface, const char *host, uint16_t port, bool bind_to_interface, int backlog);

        int accept();

        void add_to_pfds(int new_socket);

        void del_from_pfds(int old_socket);
    };
}


#endif //TCP_SERVER_H
