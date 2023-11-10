#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <random>
#include <thread>
#include "Sever.h"

std::string generateRandomString(int length) {
    const std::string characters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

    std::mt19937 rng(static_cast<unsigned int>(std::time(0)));
    std::uniform_int_distribution<> distribution(0, characters.size() - 1);

    std::string randomString;
    for (int i = 0; i < length; ++i) {
        randomString += characters[distribution(rng)];
    }

    return randomString;
}

void Server::Run() {
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
        for (const auto &room: rooms) {
            if (!room->CanJoin()) continue;
            std::cout << "Joining existing room " << room->GetName() << std::endl;
            foundRoom = true;
            room->JoinPlayer(clientSock);
        }

        std::cout << foundRoom << std::endl;
        if (!foundRoom) {
            std::string name = generateRandomString(20);
            std::cout << "Assigning new room " << name << std::endl;
            std::shared_ptr<Room> room = std::make_shared<Room>(name);
            rooms.emplace_back(room);
            room->JoinPlayer(clientSock);
            std::thread roomT(&Room::GameLoop, room);
            roomT.detach();
        }
    }
}

Server::Server(int port) {
    this->port = port;
}