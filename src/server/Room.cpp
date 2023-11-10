#include <csignal>
#include <iostream>
#include <utility>
#include "Room.h"

void Room::GameLoop() {
    int i = 0;
    while (i < 5) {
        sleep(3);
        clientMtx->lock();
        std::cout << "Slept for the: " << i << " time and got clients: " << clientCount << std::endl;
        clientMtx->unlock();
        i++;
    }
}

int Room::AvailableSpace() const {
    clientMtx->lock();
    int avail = MAX_PLAYERS - clientCount;
    clientMtx->unlock();
    return avail;
}

void Room::JoinPlayer(int fd) {
    clientMtx->lock();
    clients.push_back(fd);
    clientCount++;
    clientMtx->unlock();
}

Room::Room(std::string roomName) {
    name = std::move(roomName);
    clientMtx = std::make_unique<std::mutex>();
}
