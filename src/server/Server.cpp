#include <iostream>
#include <random>
#include <chrono>
#include <netinet/in.h>
#include <sys/socket.h>
#include "Sever.h"

void Server::Run() {
    std::cout << "Starting server" << std::endl;
    sockaddr_in localAddress{
        .sin_family = AF_INET,
        .sin_port = htons(this->port),
        .sin_addr = htonl(INADDR_ANY),
    };

    int servSock = socket(PF_INET, SOCK_STREAM, 0);
    int one = 1;
    setsockopt(servSock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));

    if(bind(servSock, (sockaddr*) &localAddress, sizeof(localAddress)))
        throw std::runtime_error("can't bind");

    listen(servSock, 1);
    std::cout << "Serving on port: " << this->port << std::endl;
    while (true) {
        int clientSock = accept(servSock, nullptr, nullptr);
        std::cout << "Got client!: " << clientSock << std::endl;
        break;
    }
}

Server::Server(int port) {
    this->port = port;
}