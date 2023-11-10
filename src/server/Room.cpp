#include <iostream>
#include <utility>
#include <thread>
#include "Room.h"

void Room::GameLoop() {
    std::cout << "Game looping!" << std::endl;

    for (int i = 0; i < 10; ++i) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::lock_guard<std::mutex> lock(clientMtx);
        std::cout << "[" << name << "]" << " got " << clientCount << " players..." << std::endl;
    }
}

bool Room::CanJoin() {
    std::lock_guard<std::mutex> lock(clientMtx);
    return MAX_PLAYERS - clientCount > 0;
}

void Room::JoinPlayer(int fd) {
    std::lock_guard<std::mutex> lock(clientMtx);
    clients.push_back(fd);
    clientCount++;
}

Room::Room(std::string roomName) {
    name = std::move(roomName);
}

std::string Room::GetName() {
    return name;
}