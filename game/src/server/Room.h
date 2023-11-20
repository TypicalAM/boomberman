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
#define GAME_WAIT_MESSAGE_INTERVAL 1000

struct AuthoredMessage {
    std::unique_ptr<GameMessage> payload;
    SPlayer *author;
};

class Room {
private:
    std::string name;
    std::vector<SBomb> bombs;
    std::vector<SPlayer> players;
    std::mutex playerMtx;
    bool gameStarted;
    int clientCount = 0;
    int64_t lastGameWaitMessage;

    std::mutex msgQueueMtx;
    std::queue<std::unique_ptr<AuthoredMessage>> msgQueue;

    int epollSock;
    std::atomic<bool> gameOver;

public:
    bool JoinPlayer(int sock, const std::string &username);

    int Players();

    bool CanJoin(const std::string &username);

    void GameLoop();

    explicit Room(std::string name);

    void ReadLoop();

    void HandleQueue();

    void HandleMessage(std::unique_ptr<AuthoredMessage> msg);

    void CheckIfGameReady();

    void HandleGameUpdates();

    static void SendSpecific(int playerSock, std::unique_ptr<GameMessage> msg);

    template<typename Function, typename ...Args>
    void SendExcept(int sock, Function &&builderFunc, Args &&... builderArgs);

    template<typename Function, typename ...Args>
    void SendBroadcast(Function &&builderFunc, Args &&... builderArgs);

    bool IsGameOver();
};

#endif