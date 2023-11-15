#include <iostream>
#include <utility>
#include <thread>
#include <fcntl.h>
#include "Room.h"
#include "../shared/Builder.h"
#include "../shared/Channel.h"

void Room::GameLoop() {
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(3));
        if (msgQueue.empty()) continue;
        auto msg = std::move(msgQueue.front()); // Acquire ownership of queue front
        msgQueue.pop();
        std::cout << "Cool message in gameloop: " << msg->message_type() << std::endl;
        msgQueueMtx->unlock();
    }
}

bool Room::CanJoin() {
    std::lock_guard<std::mutex> lock(handlerMtx);
    return MAX_PLAYERS - clientCount > 0;
}

void Room::JoinPlayer(int fd) {
    std::lock_guard<std::mutex> lock(handlerMtx);
    clientCount++;

    if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1)
        throw std::runtime_error("cannot set non-blocking mode");

    players.emplace_back(fd, "maciek", Color::RED);
    auto bytes_sent = Channel::Send(fd, Builder::GameJoin("maciek", Color::RED, true));
    if (!bytes_sent.has_value())
        throw std::runtime_error("cannot send first message");
    std::cout << "Sent the user " << bytes_sent.value() << " bytes!" << std::endl;
}

Room::Room(std::string roomName) {
    name = std::move(roomName);
    msgQueue = std::queue<std::unique_ptr<GameMessage>>();
    msgQueueMtx = std::make_shared<std::mutex>();
}