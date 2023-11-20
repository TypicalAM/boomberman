#include <iostream>
#include <utility>
#include <thread>
#include <sys/epoll.h>
#include <csignal>
#include "Room.h"
#include "../shared/Builder.h"
#include "../shared/Channel.h"
#include "../shared/Util.h"

[[noreturn]] void Room::GameLoop() {
    while (true) {
        CheckIfGameReady();
        HandleQueue();
        HandleGameUpdates();
    }
}

void Room::SendSpecific(int sock, std::unique_ptr<GameMessage> msg) {
    if (!Channel::Send(sock, std::move(msg)).has_value()) close(sock);
}

template<typename Function, typename ...Args>
void Room::SendExcept(int sock, Function &&builderFunc, Args &&... builderArgs) {
    for (auto &player: players) {
        if (player.sock == sock) continue;
        auto msg = std::invoke(std::forward<Function>(builderFunc), std::forward<Args>(builderArgs)...);
        if (!Channel::Send(player.sock, std::move(msg)).has_value()) close(player.sock);
    }
}

template<typename Function, typename ...Args>
void Room::SendBroadcast(Function &&builderFunc, Args &&... builderArgs) {
    for (auto &player: players) {
        auto msg = std::invoke(std::forward<Function>(builderFunc), std::forward<Args>(builderArgs)...);
        if (!Channel::Send(player.sock, std::move(msg)).has_value()) close(player.sock);
    }
}

bool Room::CanJoin(const std::string &username) {
    std::lock_guard<std::mutex> lock(playerMtx);
    bool already_exists;
    for (const auto &player: players) if (player.username == username) already_exists = true;
    return !already_exists && MAX_PLAYERS - clientCount > 0;
}

void Room::CheckIfGameReady() {
    std::lock_guard<std::mutex> lock(playerMtx);
    if (gameStarted) return;
    if (MAX_PLAYERS - clientCount > 0) {
        if (Util::TimestampMillis() < lastGameWaitMessage + GAME_WAIT_MESSAGE_INTERVAL) return;
        SendBroadcast(Builder::GameWait, MAX_PLAYERS - clientCount);
        lastGameWaitMessage = Util::TimestampMillis();
        return;
    }

    std::cout << "Starting game" << std::endl;
    gameStarted = true;
    SendBroadcast(Builder::GameStart);
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
            bombs.emplace_back(Coords{ipb.x(), ipb.y()}, timestamp);
            SendExcept(msg->author->sock, Builder::OtherBombPlace, timestamp, msg->author->username, ipb.x(), ipb.y());
            return;
        }

        case I_MOVE: {
            // Move the players character
            auto im = msg->payload->i_move();
            msg->author->coords.x = im.x();
            msg->author->coords.y = im.y();
            SendExcept(msg->author->sock, Builder::OtherMove, msg->author->username, im.x(), im.y());
            return;
        }

        case I_LEAVE: {
            // Notify others of the player leaving
            SendExcept(msg->author->sock, Builder::OtherLeave, msg->author->username);
            close(msg->author->sock);
            return;
        }

        default:
            std::cout << "Unexpected message type: " << msg->payload->message_type() << std::endl;
            SendSpecific(msg->author->sock, Builder::Error("Unexpected message"));
    }
}

[[noreturn]] void Room::ReadLoop() {
    epoll_event events[10];
    while (true) {
        // Wait for events to occur
        int event_num = epoll_wait(epollSock, events, 10, -1);
        for (int i = 0; i < event_num; ++i) {
            // If this isn't an in event, ignore it
            if (!(events[i].events & EPOLLIN)) {
                std::cout << "Ignoring event on socket: " << events[i].data.fd;
                continue;
            }

            // We got a message from a client
            std::lock_guard<std::mutex> lock(playerMtx);
            auto client_sock = events[i].data.fd;
            auto msg = Channel::Receive(client_sock);
            if (!msg.has_value()) {
                std::cout << "Closing connection since we can't receive data: " << client_sock << std::endl;
                epoll_ctl(epollSock, EPOLL_CTL_DEL, client_sock, nullptr);
                close(client_sock);

                int author_idx = -1;
                for (int j = 0; j < players.size(); ++j) if (players[j].sock == client_sock) author_idx = j;
                if (author_idx == -1) continue; // Look below

                SendExcept(players[author_idx].sock, Builder::OtherLeave, players[author_idx].username);
                players.erase(players.begin() + author_idx);
                continue;
            }

            // Find the player and associate it with the message
            SPlayer *author;
            for (auto &player: players) if (player.sock == client_sock) author = &player;
            if (author == nullptr) {
                // This happens when a player has been disconnected from the game
                // (kicked out, idk) but they still somehow sent a message here. In that
                // case broadcasting of their leaving should've already been handled above.
                std::cout << "Closing connection since there isn't a player like that: " << client_sock << std::endl;
                epoll_ctl(epollSock, EPOLL_CTL_DEL, client_sock, nullptr);
                close(client_sock);
                continue;
            }

            // Push the authored message into the queue
            std::lock_guard<std::mutex> queue_lock(msgQueueMtx);
            msgQueue.push(std::make_unique<AuthoredMessage>(AuthoredMessage{std::move(msg.value()), author}));
        }
    }
}

bool Room::JoinPlayer(int sock, const std::string &username) {
    std::lock_guard<std::mutex> lock(playerMtx);
    clientCount++;
    auto color = static_cast<Color>(players.size());
    if (!Channel::Send(sock, Builder::GameJoin(username, color, true)).has_value()) return false;
    epoll_event event = {EPOLLIN | EPOLLET, epoll_data{.fd = sock}};
    if (epoll_ctl(epollSock, EPOLL_CTL_ADD, sock, &event) == -1) throw std::runtime_error("cannot add to epoll");
    players.emplace_back(sock, username, color);
    return true;
}

Room::Room(std::string roomName) {
    gameStarted = false;
    name = std::move(roomName);
    msgQueue = std::queue<std::unique_ptr<AuthoredMessage>>();
    lastGameWaitMessage = Util::TimestampMillis();

    // Create epoll instance
    if ((epollSock = epoll_create1(0)) == -1) {
        close(epollSock); // Clean up on failure
        throw std::runtime_error("epoll creation failed");
    }
}

int Room::Players() {
    std::lock_guard<std::mutex> lock(playerMtx);
    return clientCount;
}