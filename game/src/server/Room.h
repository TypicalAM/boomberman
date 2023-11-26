#ifndef BOOMBERMAN_ROOM_H
#define BOOMBERMAN_ROOM_H

#include <vector>
#include <mutex>
#include <memory>
#include <queue>
#include "../shared/messages.pb.h"
#include "entities/SPlayer.h"
#include "../shared/game/Map.h"
#include "../shared/game/Bomb.h"
#include <boost/log/sources/logger.hpp>
#include <boost/log/sources/record_ostream.hpp>

#define MAX_PLAYERS 4
#define GAME_WAIT_MESSAGE_INTERVAL 1000
#define LOG BOOST_LOG(this->logger)

enum GameState {
    WAIT_FOR_START,
    PLAY,
    WAIT_FOR_END,
    GAME_OVER,
};

struct AuthoredMessage {
    std::unique_ptr<GameMessage> payload;
    std::shared_ptr<SPlayer> author;
};

class Room {
private:
    std::string name;
    std::vector<Bomb> bombs;
    std::atomic<GameState> state;
    int64_t lastGameWaitMessage;

    std::mutex playerMtx;
    std::vector<std::shared_ptr<SPlayer>> players;
    int clientCount = 0;

    std::mutex msgQueueMtx;
    std::queue<std::unique_ptr<AuthoredMessage>> msgQueue;
    boost::log::sources::logger logger;

    std::unique_ptr<Map> map;

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

    void ReadIntoQueue();

public:
    int Players();

    bool CanJoin(const std::string &username);

    void GameLoop();

    bool JoinPlayer(int sock, const std::string &username);

    bool IsGameOver();

    Room(boost::log::sources::logger logger, std::string name);
};

#endif