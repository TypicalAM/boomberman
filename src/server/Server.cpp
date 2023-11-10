#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <random>
#include <thread>
#include "Sever.h"

// Thanks https://stackoverflow.com/questions/440133/how-do-i-create-a-random-alpha-numeric-string-in-c
std::string random_string(std::string::size_type length) {
    static auto &chrs = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    thread_local static std::mt19937 rg{std::random_device{}()};
    thread_local static std::uniform_int_distribution<std::string::size_type> pick(0, sizeof(chrs) - 2);

    std::string result;
    result.reserve(length);
    while (length--) result += chrs[pick(rg)];
    return result;
}

[[noreturn]] void Server::Run() {
    std::cout << "Starting server" << std::endl;
    sockaddr_in localAddress{
            .sin_family = AF_INET,
            .sin_port = htons(this->port),
            .sin_addr = {htonl(INADDR_ANY)},
    };

    int servSock = socket(PF_INET, SOCK_STREAM, 0);
    int one = 1;
    setsockopt(servSock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));

    if (bind(servSock, (sockaddr *) &localAddress, sizeof(localAddress)))
        throw std::runtime_error("can't bind");

    listen(servSock, 1);
    std::cout << "Serving on port: " << this->port << std::endl;

    while (true) {
        std::cout << "Waiting for client" << std::endl;
        int clientSock = accept(servSock, nullptr, nullptr);
        std::cout << "Got client!: " << clientSock << std::endl;

        bool foundRoom = false;
        for (const auto &pair: rooms) {
            if (!pair.second->CanJoin()) continue;
            std::cout << "Joining existing room " << pair.first << std::endl;
            foundRoom = true;
            pair.second->JoinPlayer(clientSock);
        }

        std::cout << foundRoom << std::endl;
        if (!foundRoom) {
            std::string name = random_string(10);
            std::cout << "Assigning new room " << name << std::endl;
            std::shared_ptr<Room> room = std::make_shared<Room>(name);
            rooms[name] = room;
            room->JoinPlayer(clientSock);
            std::thread roomT(&Room::GameLoop, room);
            roomT.detach();
        }
    }
}

Server::Server(int port) {
    this->port = port;
}