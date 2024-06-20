#include <iostream>
#include <thread>
#include <iomanip>
#include "Server.h"
#include "Client.h"

void server_loop() {
    tcp::Server _server;

    if(0 == _server.setup("enp5s0", "127.0.0.1", 12345, false, 50) ) {
        while(auto client = _server.accept()) {
            if(client > 0) {
                send(client, "hi and bye", 10, 0);
                close(client);
            }
            return;
        }
    }
}

int main() {
    std::cout << "Hello, World!" << std::endl;

    std::thread server_thread (server_loop);


    tcp::Client _client("127.0.0.1", 12345);
    unsigned char mac_addr[6]; // no null terminated!
    tcp::Client::getMacAddress("enp5s0", mac_addr);

    //std::cout << std::setw(2) << std::setfill('0') << std::hex << std::uppercase << mac_addr << "\n";
    int i = 0;
    printf("MAC: %02X:%02X:%02X:%02X:%02X:%02X\n", mac_addr[i++], mac_addr[i++], mac_addr[i++], mac_addr[i++], mac_addr[i++], mac_addr[i]);

    char message[15];
    _client.connect();
    int numbytes = _client.recv(message, 14);
    printf("numbytes: %i\n", numbytes);
    printf("message: %s\n", message);

    if(_client.isConnected()) {
        printf("still think I'm connected still -> shutting down\n");
        _client.shutdown();
    }

    server_thread.join();


    return 0;
}
