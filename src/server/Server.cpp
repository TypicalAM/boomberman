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

std::optional<int> Server::setup() {
    sockaddr_in localAddress{AF_INET, htons(this->port), htonl(INADDR_ANY)};
    int servSock = socket(PF_INET, SOCK_STREAM, 0);
    int one = 1;
    setsockopt(servSock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    if (bind(servSock, (sockaddr *) &localAddress, sizeof(localAddress))) return std::nullopt;
    listen(servSock, 1);
    return servSock;
}

[[noreturn]] void Server::Run() {
    std::cout << "Serving on port: " << this->port << std::endl;
    std::optional<int> serv = setup();
    if (!serv.has_value())
        throw std::runtime_error("failed to set up the tcp server");

    while (true) {
        std::cout << "Waiting for client" << std::endl;
        int clientSock = accept(serv.value(), nullptr, nullptr);
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
            std::thread roomT(&Room::GameLoop, room);
            room->JoinPlayer(clientSock);
            roomT.detach();
        }
    }
}

Server::Server(int port) {
    this->port = port;
}