#include <iostream>
#include <utility>
#include <thread>
#include "Room.h"
#include "ClientHandler.h"

void Room::GameLoop() {
    std::cout << "Game looping!" << std::endl;

    for (int i = 0; i < 10; ++i) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::lock_guard<std::mutex> lock(handlerMtx);
        std::cout << "[" << name << "]" << " got " << clientCount << " players..." << std::endl;
    }
}

bool Room::CanJoin() {
    std::lock_guard<std::mutex> lock(handlerMtx);
    return MAX_PLAYERS - clientCount > 0;
}

void Room::JoinPlayer(int fd) {
    std::lock_guard<std::mutex> lock(handlerMtx);
    auto handler = std::make_shared<ClientHandler>(fd, msgQueue, msgQueueMtx);
    std::thread(&ClientHandler::ReadLoop, handler).detach();
    handlers.push_back(handler);
    clientCount++;
    std::cout << "Started a new read thread for user " << fd << std::endl;
}

Room::Room(std::string roomName) {
    name = std::move(roomName);
    msgQueue = std::make_shared<std::queue<Message>>();
    msgQueueMtx = std::make_shared<std::mutex>();
}