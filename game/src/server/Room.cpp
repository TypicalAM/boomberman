#include <iostream>
#include <utility>
#include <thread>
#include <fcntl.h>
#include "Room.h"
#include "../shared/Builder.h"
#include "../shared/Channel.h"

[[noreturn]] void Room::GameLoop() {
    while (true) {
        ReadIntoQueue();
        HandleQueue();
    }
}

bool Room::CanJoin() {
    std::lock_guard<std::mutex> lock(handlerMtx);
    return MAX_PLAYERS - clientCount > 0;
}

void Room::HandleQueue() {
    std::lock_guard<std::mutex> lock(msgQueueMtx);
    while (!msgQueue.empty()) {
        HandleMessage(std::move(msgQueue.front()));
        msgQueue.pop();
    }
}

void Room::HandleMessage(std::unique_ptr<GameMessage> msg) {
    switch (msg->message_type()) {
        case I_PLACE_BOMB: {
            break;
        }

        case I_MOVE: {
            break;
        }

        case I_LEAVE: {
            break;
        }

        default:
            throw std::runtime_error("server-side message from client");
    }
}

void Room::ReadIntoQueue() {
    std::lock_guard<std::mutex> lock(handlerMtx);
    for (const auto &player: players) {
        auto msg = Channel::Receive(player.sock);
        if (!msg.has_value()) continue;
        std::lock_guard<std::mutex> q_lock(msgQueueMtx);
        msgQueue.push(std::move(msg.value()));
    }
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
}