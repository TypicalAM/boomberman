#include <iostream>
#include <utility>
#include <thread>
#include "Room.h"
#include "../shared/Builder.h"

void Room::GameLoop() {
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(3));
        msgQueueMtx->lock();
        if (msgQueue->empty()) {
            msgQueueMtx->unlock();
            continue;
        }

        auto msg = std::move(msgQueue->front()); // Acquire ownership of queue front
        msgQueue->pop();
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
    auto handler = std::make_shared<ClientHandler>(fd, msgQueue, msgQueueMtx);
    std::thread(&ClientHandler::ReadLoop, handler).detach();
    handlers.push_back(handler);
    clientCount++;

    std::cout << "Started a new read thread for user " << fd << std::endl;
    auto bytes_sent = Channel::Send(fd, Builder::GameJoin("maciek", Color::RED, true));
    if (!bytes_sent.has_value())
        throw std::runtime_error("cannot send first message");
    std::cout << "Sent the user " << bytes_sent.value() << " bytes!" << std::endl;
}

Room::Room(std::string roomName) {
    name = std::move(roomName);
    msgQueue = std::make_shared<std::queue<std::unique_ptr<GameMessage>>>();
    msgQueueMtx = std::make_shared<std::mutex>();
}