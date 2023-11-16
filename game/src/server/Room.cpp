#include <iostream>
#include <utility>
#include <thread>
#include <fcntl.h>
#include "Room.h"
#include "../shared/Builder.h"
#include "../shared/Channel.h"
#include "../shared/Util.h"

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
    switch (msg->payload->message_type()) {
        case I_PLACE_BOMB: {
            // Place the bomb at the specified location
            IPlaceBombMsg ipb = msg->payload->i_place_bomb();
            int64_t timestamp = Util::TimestampMillis();
            bombs.emplace_back(Coords{.x = ipb.x(), .y = ipb.y()}, timestamp);
            for (const auto &player: players) {
                if (player.username == msg->author->username) continue;
                auto new_msg = Builder::OtherBombPlace(timestamp, msg->author->username, ipb.x(), ipb.y());
                auto bytes_sent = Channel::Send(player.sock, std::move(new_msg));
                if (!bytes_sent.has_value()) throw std::runtime_error("can't broadcast message to clients");
            }
            return;
        }

        case I_MOVE: {
            // Move the players character
            auto im = msg->payload->i_move();
            msg->author->coords.x = im.x();
            msg->author->coords.y = im.y();
            for (const auto &player: players) {
                if (player.username == msg->author->username) continue;
                auto bytes_sent = Channel::Send(player.sock, Builder::OtherMove(msg->author->username, im.x(), im.y()));
                if (!bytes_sent.has_value()) throw std::runtime_error("can't broadcast message to clients");
            }
            return;
        }

        case I_LEAVE: {
            // Notify others of the player leaving
            for (auto it = players.begin(); it != players.end(); ++it) if (&(*it) == msg->author) players.erase(it);
            for (const auto &player: players) {
                if (player.username == msg->author->username) continue;
                auto bytes_sent = Channel::Send(player.sock, Builder::OtherLeave(msg->author->username));
                if (!bytes_sent.has_value()) throw std::runtime_error("can't broadcast message to clients");
            }
            return;
        }

        default:
            throw std::runtime_error("server-side message from client");
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
    if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1) throw std::runtime_error("cannot set non-blocking mode");

    auto color = static_cast<Color>(players.size());
    auto username = Util::RandomString(5); // TODO: Check if there is a collision (or just increase length lol)
    auto bytes_sent = Channel::Send(fd, Builder::GameJoin(username, color, true));
    if (!bytes_sent.has_value()) throw std::runtime_error("cannot send first message");
    players.emplace_back(fd, username, color);
}

Room::Room(std::string roomName) {
    name = std::move(roomName);
    msgQueue = std::queue<std::unique_ptr<AuthoredMessage>>();
}