//
// Created by zysik on 17.11.20.
//

#ifndef TCP_TCPSTRUCTS_H
#define TCP_TCPSTRUCTS_H

namespace tcp {
    struct KeepConfig {
        //! The time (in seconds) the connection needs to remain idle before TCP starts sending keepalive probes (TCP_KEEPIDLE socket option)
        int keepidle;
        //! The maximum number of keepalive probes TCP should send before dropping the connection. (TCP_KEEPCNT socket option)
        int keepcnt;
        //! The time (in seconds) between individual keepalive probes. (TCP_KEEPINTVL socket option)
        int keepintvl;
    };
}

#endif //TCP_TCPSTRUCTS_H
