//
// Created by zysik on 17.11.20.
//

#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H

/// @cond
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <net/if.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <cstring>
#include <string>
#include <vector>
#include <chrono>
#include "tcpStructs.h"
/// @endcond


namespace tcp {

//!  handle basic TCP client socket tasks
/*!
  simple tcp networking client class for linux based systems to connect to
  a server. There is no async calling happening in here.
*/
    class Client {
    private:
        //! hostname or ip address to connect to.
        /*!
            used as sockaddr_in.sin_addr.s_addr = inet_addr(host_)
        */
        const char *host_;

        //! port to connect to.
        /*!
            used as sockaddr_in.sin_port = htons(port_)
        */
        unsigned short port_;

        //! socket descriptor
        int socket_desc_;

        //! flag reporting last known status of connection
        bool is_connected_;

        //! enable TCP keepalive on the socket
        /*!
            enable TCP keepalive on the socket,
            called on setting keepalive_cfg
            \param sockfd file descriptor
            \return 0 on success -1 on failure ( the value of setsockopt call )
        */
        static int set_tcp_keepalive(int sockfd);

    public:

        //! constructor defining host_ and port_.
        /*!
            defaults socket_desc_ to 0 and is_connected_ to false
        */
        Client(const char *host, uint16_t port);

        //! destructor.
        /*!
            calls shutdown()
        */
        ~Client();

        //! closes open socket.
        /*!
            if socket_descriptor is greater 0 shutdown and close on socket_descriptor is called
            socket_descriptor is set to -1
            is_connected_ is set to false
        */
        void shutdown();

        //! receive data from network buffer
        /*!
            if is_connected_ is true, recv function is called on socket_descriptor
                with the provided buffer and size information.
                result of recv call is returned, if the result is LT 0 is_connected_ is set to false

            otherwise -1 is returned
            \param buffer char buffer to write received data to
            \param size describing the size of the buffer / max amount of data to be fetched .
            \return result negative value describing an error, positive value describing bytes read from network
        */
        ssize_t recv(void *buffer, size_t size);

        //! returns connection status
        /*!
            first the shutdown function is called to ensure we do not have an active descriptor left somewhere
            create new AF_INET / SOCK_STREAM proto 0 socket, if this worked
               set SO_REUSEADDR
               set keepalive config { 30,5,5 }
               call ::connect
               set is_connected_ to true if there was no error

            return is_connected_ status which is false due to shutdown() call on error or true if ::connect succeeded

            // TODO allow to set own KeepConfig before calling connect
            \return is_connected_ last known state of connection
         */
        bool connect();

        //! returns connection status
        /*!
            if is_connected_ is set to true, the socket is asked if any error is known,
            if we have an error_code is_connected_ is set to false otherwise
            attempt to send data and save the result.
            If the result is negative, is_connected_ is set to false.

            \param message char buffer to send to destination
            \param numbytes amount of bytes to read from message, be aware to check your message size
            \return -1 if we were not connected or the error code if any otherwise result of ::send function call
         */
        ssize_t send(const char *message, uint64_t numbytes);

        //! returns connection status
        /*!
            return is_connected_ value, might get updated on send/recv call,
            could return "true" even the connection is already closed in reality.
            \return is_connected_ last known state of connection
         */
        [[nodiscard]] bool isConnected() const;

        //! read MAC from network interface
        /*!
            saving the MAC of a network interface into dest buffer,
            you have to make sure dest buffer is at least 6 bytes in size.

            This function temporarily creates a new socket on that interface.
            \param dev the device name
            \param dest destination buffer to save MAC in
         */
        static void getMacAddress(const char *dev, unsigned char *dest);

        //! Set the keepalive options on the socket
        /*!
            first try to enable keepalive on socket ( SO_KEEPALIVE ) on success
            also set the other options ( TCP_KEEPCNT, TCP_KEEPIDLE, TCP_KEEPINTVL )

            \param sockfd file descriptor
            \param cfg configuration to apply on socket
            \return 0 on success or  return value of last setsockopt call
        */
        static int set_tcp_keepalive_cfg(int sockfd, const struct KeepConfig *cfg);

    };

}

#endif //TCP_CLIENT_H
