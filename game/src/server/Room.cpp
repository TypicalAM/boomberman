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

void Room::HandleMessage(std::unique_ptr<AuthoredMessage> msg) {
    std::cout << "Handling a message from user: " << msg->author->username << std::endl;
    std::unique_ptr<GameMessage> to_broadcast;

    switch (msg->payload->message_type()) {
        case I_PLACE_BOMB: {
            // Place the bomb at the specified location
            IPlaceBombMsg ipb = msg->payload->i_place_bomb();
            int64_t timestamp = static_cast<int64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch()).count()); // TODO: Put somewhere else
            bombs.emplace_back(Coords{.x = ipb.x(), .y = ipb.y()}, timestamp);
            to_broadcast = Builder::OtherBombPlace(timestamp, msg->author->username, ipb.x(), ipb.y());
            break;
        }

        case I_MOVE: {
            // Move the players character
            auto im = msg->payload->i_move();
            msg->author->coords.x = im.x();
            msg->author->coords.y = im.y();
            to_broadcast = Builder::OtherMove(msg->author->username, im.x(), im.y());
            break;
        }

        case I_LEAVE: {
            // Notify others of the player leaving
            // TODO: Delete the person
            to_broadcast = Builder::OtherLeave(msg->author->username);
            break;
        }

        default:
            throw std::runtime_error("server-side message from client");
    }

    // Broadcast the message to everyone except the sender
    for (const auto &player: players) {
        if (player.username == msg->author->username) continue;
        auto bytes_sent = Channel::Send(player.sock, std::move(to_broadcast));
        if (!bytes_sent.has_value()) throw std::runtime_error("can't broadcast message to clients");
    }
}

void Room::ReadIntoQueue() {
    std::lock_guard<std::mutex> lock(handlerMtx);
    for (int i = 0; i < players.size(); ++i) {
        auto msg = Channel::Receive(players[i].sock);
        if (!msg.has_value()) continue;
        std::lock_guard<std::mutex> q_lock(msgQueueMtx);
        msgQueue.push(std::make_unique<AuthoredMessage>(AuthoredMessage{
                .payload = std::move(msg.value()),
                .author = &players[i],
        }));
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
    msgQueue = std::queue<std::unique_ptr<AuthoredMessage>>();
}