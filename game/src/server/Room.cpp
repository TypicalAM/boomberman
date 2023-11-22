#include <iostream>
#include <utility>
#include <thread>
#include <sys/epoll.h>
#include <csignal>
#include "Room.h"
#include "../shared/Builder.h"
#include "../shared/Channel.h"
#include "../shared/Util.h"

void Room::GameLoop() {
    while (state.load() != GAME_OVER) {
        ReadIntoQueue();
        CheckIfGameReady();
        HandleQueue();
        HandleGameUpdates();
    }

    std::cout << "Gameloop done on room: " << name << std::endl;
}

void Room::SendSpecific(int sock, std::unique_ptr<GameMessage> msg) {
    if (!Channel::Send(sock, std::move(msg)).has_value()) close(sock);
}

template<typename Function, typename ...Args>
void Room::SendExcept(int sock, Function &&builderFunc, Args &&... builderArgs) {
    for (auto &player: players) {
        if (player->sock == sock) continue;
        auto msg = std::invoke(std::forward<Function>(builderFunc), std::forward<Args>(builderArgs)...);
        if (!Channel::Send(player->sock, std::move(msg)).has_value()) close(player->sock);
    }
}

template<typename Function, typename ...Args>
void Room::SendBroadcast(Function &&builderFunc, Args &&... builderArgs) {
    for (auto &player: players) {
        auto msg = std::invoke(std::forward<Function>(builderFunc), std::forward<Args>(builderArgs)...);
        if (!Channel::Send(player->sock, std::move(msg)).has_value()) close(player->sock);
    }
}

bool Room::CanJoin(const std::string &username) {
    std::lock_guard<std::mutex> lock(playerMtx);
    bool already_exists = false;
    for (const auto &player: players) if (player->username == username) already_exists = true;
    return !already_exists && MAX_PLAYERS - clientCount > 0;
}

void Room::CheckIfGameReady() {
    if (state.load() != WAIT_FOR_START) return;

    std::lock_guard<std::mutex> lock(playerMtx);
    if (MAX_PLAYERS - clientCount > 0) {
        if (Util::TimestampMillis() < lastGameWaitMessage + GAME_WAIT_MESSAGE_INTERVAL) return;
        SendBroadcast(Builder::GameWait, MAX_PLAYERS - clientCount);
        lastGameWaitMessage = Util::TimestampMillis();
        return;
    }

    std::cout << "Starting game" << std::endl;
    state.store(PLAY);
    std::vector<std::string> usernames;
    for (const auto &player: players) usernames.push_back(player->username);
    SendBroadcast(Builder::GameStart, usernames);
}

void Room::HandleGameUpdates() {
    if (state.load() != PLAY) return;

    std::lock_guard<std::mutex> lock(playerMtx);
    std::vector<int> explode_indices;

    for (int i = 0; i < bombs.size(); i++) {
        if (!bombs[i].ShouldExplode()) continue;
        explode_indices.push_back(i);

        // The bomb explodes
        for (auto &player: players)
            if (bombs[i].InBlastRadius(player)) {
                player->livesRemaining--;
                SendBroadcast(Builder::GotHit, player->username, player->livesRemaining);
            }
    }

    for (const auto &index: explode_indices)
        bombs.erase(bombs.begin() + index);

    int people_alive = 0;
    int last_alive_index = -1;
    for (int i = 0; i < players.size(); ++i) {
        if (players[i]->livesRemaining != 0) {
            people_alive++;
            last_alive_index = i;
        }
    }

    if (people_alive == 1 && state.load() == PLAY) {
        // This person has won the game, announce that and disconnect all clients
        std::cout << "Player won by last person alive: " << players[last_alive_index]->username << std::endl;
        SendBroadcast(Builder::GameWon, players[last_alive_index]->username);
        state.store(WAIT_FOR_END);
        std::this_thread::sleep_for(std::chrono::seconds(3));
        std::cout << "Closing other connections and ending the game" << std::endl;
        for (auto &player: players) close(player->sock);
        state.store(GAME_OVER);
        close(epollSock);
    }
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

    // If the game started
    if (state.load() == WAIT_FOR_START) {
        SendSpecific(msg->author->sock, Builder::Error("Game hasn't started yet"));
        return;
    }

    // Check if the player is playing tricks
    if (!msg->author->livesRemaining && msg->payload->message_type() != I_LEAVE) {
        SendSpecific(msg->author->sock, Builder::Error("You can't really do anything while dead, can you?"));
        return;
    }

    // Handle message based on the type
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
            std::lock_guard<std::mutex> lock(playerMtx);
            if (players.size() == 1) {
                close(players[0]->sock);
                state.store(GAME_OVER); // Close the game
                close(epollSock);
                return;
            }

            if (players.size() == 2 && state.load() == PLAY) {
                // This player just won lol, let's notify the last one standing
                auto winner_idx = (players[0]->sock == msg->author->sock) ? 1 : 0;
                std::cout << "Player won by other disconnecting: " << players[winner_idx]->username << std::endl;
                SendBroadcast(Builder::GameWon, players[winner_idx]->username);
                state.store(WAIT_FOR_END);
                std::this_thread::sleep_for(std::chrono::seconds(3));
                std::cout << "Closing other connections and ending the game" << std::endl;
                for (auto &player: players) close(player->sock);
                state.store(GAME_OVER);
                close(epollSock);
                return;
            }

            SendExcept(msg->author->sock, Builder::OtherLeave, msg->author->username);
            close(msg->author->sock);
            int author_idx = -1;
            for (int i = 0; i < players.size(); ++i) if (players[i]->sock == msg->author->sock) author_idx = i;
            players.erase(players.begin() + author_idx);
            return;
        }

        default:
            std::cout << "Unexpected message type: " << msg->payload->message_type() << std::endl;
            SendSpecific(msg->author->sock, Builder::Error("Unexpected message"));
    }
}

void Room::ReadIntoQueue() {
    if (state.load() == GAME_OVER) return;

    epoll_event events[10];
    int event_num = epoll_wait(epollSock, events, 10, 0);
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
            for (int j = 0; j < players.size(); ++j) if (players[j]->sock == client_sock) author_idx = j;
            if (author_idx == -1) continue; // Look below

            SendExcept(players[author_idx]->sock, Builder::OtherLeave, players[author_idx]->username);
            players.erase(players.begin() + author_idx);
            continue;
        }

        // Find the player and associate it with the message
        int author_idx = -1;
        for (int j = 0; j < players.size(); ++j) if (players[j]->sock == client_sock) author_idx = j;
        if (author_idx == -1) {
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
        msgQueue.push(
                std::make_unique<AuthoredMessage>(AuthoredMessage{std::move(msg.value()), players[author_idx]}));
    }
}

bool Room::JoinPlayer(int sock, const std::string &username) {
    if (state.load() != WAIT_FOR_START) return false;
    std::lock_guard<std::mutex> lock(playerMtx);
    clientCount++;
    auto color = static_cast<Color>(players.size());
    if (!Channel::Send(sock, Builder::GameJoin(username, color, true)).has_value()) return false;
    epoll_event event = {EPOLLIN | EPOLLET, epoll_data{.fd = sock}};
    if (epoll_ctl(epollSock, EPOLL_CTL_ADD, sock, &event) == -1) throw std::runtime_error("cannot add to epoll");
    players.emplace_back(std::make_unique<SPlayer>(sock, username, color));
    return true;
}

Room::Room(std::string roomName) {
    name = std::move(roomName);
    state.store(WAIT_FOR_START);
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

bool Room::IsGameOver() {
    return state.load() == GAME_OVER;
}