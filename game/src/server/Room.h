#ifndef BOOMBERMAN_ROOM_H
#define BOOMBERMAN_ROOM_H

#include <vector>
#include <mutex>
#include <memory>
#include <queue>
#include "../shared/messages.pb.h"
#include "entities/SPlayer.h"
#include "entities/SBomb.h"

#define MAX_PLAYERS 4

struct AuthoredMessage {
    std::unique_ptr<GameMessage> payload;
    std::string author;
};

class Room {
private:
    std::string name;
    std::vector<SBomb> bombs;
    std::vector<SPlayer> players;
    std::mutex handlerMtx;
    int clientCount = 0;

    std::mutex msgQueueMtx;
    std::queue<std::unique_ptr<AuthoredMessage>> msgQueue;

public:
    void JoinPlayer(int fd);

    bool CanJoin();

    [[noreturn]] void GameLoop();

    explicit Room(std::string name);

    void ReadIntoQueue();

    void HandleQueue();

    void HandleMessage(std::unique_ptr<AuthoredMessage> msg);
};

#endif