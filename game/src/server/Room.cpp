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
        CheckIfGameReady();
        HandleQueue();
        HandleGameUpdates();
    }
}

bool Room::CanJoin(const std::string &username) {
    std::lock_guard<std::mutex> lock(handlerMtx);
    bool already_exists;
    for (const auto &player: players) if (player.username == username) already_exists = true;
    return already_exists || MAX_PLAYERS - clientCount > 0;
}

void Room::CheckIfGameReady() {
    if (gameStarted) return;
    if (MAX_PLAYERS - clientCount > 0) {
        if (Util::TimestampMillis() < lastGameWaitMessage + GAME_WAIT_MESSAGE_INTERVAL) return;
        for (const auto &player: players) {
            auto bytes_sent = Channel::Send(player.sock, Builder::GameWait(MAX_PLAYERS - clientCount));
            if (!bytes_sent.has_value()) throw std::runtime_error("can't send game wait msg");
        }

        lastGameWaitMessage = Util::TimestampMillis();
        return;
    }

    std::cout << "Starting game" << std::endl;
    gameStarted = true;
    for (const auto &player: players) {
        auto bytes_sent = Channel::Send(player.sock, Builder::GameStart());
        if (!bytes_sent.has_value()) throw std::runtime_error("can't send game start msg");
    }
}

void Room::HandleGameUpdates() {
    std::vector<int> explode_indices;

    for (int i = 0; i < bombs.size(); i++) {
        if (!bombs[i].ShouldExplode()) continue;
        explode_indices.push_back(i);

        // The bomb explodes
        for (auto &player: players)
            if (bombs[i].InBlastRadius(player)) {
                player.livesRemaining--;
                for (const auto &player2: players)
                    Channel::Send(player2.sock, Builder::GotHit(player.username, player.livesRemaining));
            }
    }

    for (const auto &index: explode_indices)
        bombs.erase(bombs.begin() + index);
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

void Room::JoinPlayer(int fd, const std::string &username) {
    std::lock_guard<std::mutex> lock(handlerMtx);
    clientCount++;
    auto color = static_cast<Color>(players.size());
    auto bytes_sent = Channel::Send(fd, Builder::GameJoin(username, color, true));
    if (!bytes_sent.has_value()) throw std::runtime_error("cannot send first message");
    players.emplace_back(fd, username, color);
}

Room::Room(std::string roomName) {
    gameStarted = false;
    name = std::move(roomName);
    msgQueue = std::queue<std::unique_ptr<AuthoredMessage>>();
    lastGameWaitMessage = Util::TimestampMillis();
}

int Room::Players() {
    std::lock_guard<std::mutex> lock(handlerMtx);
    return clientCount;
}