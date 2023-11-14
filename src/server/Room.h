#ifndef BOOMBERMAN_ROOM_H
#define BOOMBERMAN_ROOM_H

#include <vector>
#include <mutex>
#include <memory>
#include <queue>
#include "ClientHandler.h"

#define MAX_PLAYERS 2

class Room {
private:
    std::string name;
    std::vector<std::shared_ptr<ClientHandler>> handlers;
    std::mutex handlerMtx;
    int clientCount = 0;

    std::shared_ptr<std::mutex> msgQueueMtx;
    std::shared_ptr<std::queue<std::unique_ptr<GameMessage>>> msgQueue;

public:
    void JoinPlayer(int fd);

    bool CanJoin();

    void GameLoop();

    explicit Room(std::string name);
};

#endif