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

enum GameState {
    WAIT_FOR_START,
    PLAY,
    WAIT_FOR_END,
    GAME_OVER,
};

struct AuthoredMessage {
    std::unique_ptr<GameMessage> payload;
    SPlayer *author;
};

class Room {
private:
    std::string name;
    std::vector<SBomb> bombs;
    std::atomic<GameState> state;
    int64_t lastGameWaitMessage;

    std::mutex playerMtx;
    std::vector<SPlayer> players;
    int clientCount = 0;

    std::mutex msgQueueMtx;
    std::queue<std::unique_ptr<AuthoredMessage>> msgQueue;

    int epollSock;

    void HandleQueue();

    void HandleMessage(std::unique_ptr<AuthoredMessage> msg);

    void CheckIfGameReady();

    void HandleGameUpdates();

    static void SendSpecific(int playerSock, std::unique_ptr<GameMessage> msg);

    template<typename Function, typename ...Args>
    void SendExcept(int sock, Function &&builderFunc, Args &&... builderArgs);

    template<typename Function, typename ...Args>
    void SendBroadcast(Function &&builderFunc, Args &&... builderArgs);

public:
    int Players();

    void ReadLoop();

    bool CanJoin(const std::string &username);

    void GameLoop();

    bool JoinPlayer(int sock, const std::string &username);

    bool IsGameOver();

    explicit Room(std::string name);
};

#endif