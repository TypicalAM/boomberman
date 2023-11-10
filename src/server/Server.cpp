#include <iostream>
#include "Sever.h"

void Server::Run() {
    std::cout << "Serving at port: " << this->port << std::endl;
}

Server::Server(int port) {
    this->port = port;
}