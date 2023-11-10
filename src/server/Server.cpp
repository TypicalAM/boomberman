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
            .sin_addr = htonl(INADDR_ANY),
    };

    int servSock = socket(PF_INET, SOCK_STREAM, 0);
    int one = 1;
    setsockopt(servSock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));

    if (bind(servSock, (sockaddr *) &localAddress, sizeof(localAddress)))
        throw std::runtime_error("can't bind");

    listen(servSock, 1);
    std::cout << "Serving on port: " << this->port << std::endl;

    while (true) {
        int clientSock = accept(servSock, nullptr, nullptr);
        std::cout << "Got client!: " << clientSock << std::endl;
        bool foundRoom;

        for (auto &pair: rooms) {
            if (pair.second->AvailableSpace() > 0) {
                foundRoom = true;
                pair.second->JoinPlayer(clientSock);
                std::cout << "Joining player to room:" << pair.first << std::endl;
            }
        }

        if (!foundRoom) {
            // Let's create a new room
            std::string name = generateRandomString(10);
            Room myRoom(name);
            std::thread roomT(&Room::GameLoop, &myRoom);
            rooms[name] = std::make_unique<Room>(name);
            std::cout << "Created a new room with name: " << name << std::endl;
            roomT.join();
        }
    }
}

Server::Server(int port) {
    this->port = port;
}