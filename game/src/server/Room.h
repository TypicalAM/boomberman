#ifndef BOOMBERMAN_ROOM_H
#define BOOMBERMAN_ROOM_H

#include <vector>
#include <mutex>
#include <memory>
#include <queue>
#include "../shared/proto/messages.pb.h"
#include "entities/SPlayer.h"
#include "../shared/game/Map.h"
#include "../shared/game/Bomb.h"
#include "../shared/proto/messages.pb.h"
#include <boost/log/sources/logger.hpp>
#include <boost/log/sources/record_ostream.hpp>

#define MAX_PLAYERS 3
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
    std::vector<Bomb> bombs;
    std::atomic<GameState> state = WAIT_FOR_START;
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

    void SendSpecific(Connection conn, std::unique_ptr<GameMessage> msg);

    template<typename Function, typename ...Args>
    void SendExcept(Connection conn, Function &&builderFunc, Args &&... builderArgs);

    template<typename Function, typename ...Args>
    void SendBroadcast(Function &&builderFunc, Args &&... builderArgs);

    void ReadIntoQueue();

public:
    int Players();

    bool CanJoin(const std::string &username);

    void GameLoop();

    bool JoinPlayer(Connection conn, const std::string &username);

    bool IsGameOver();

    explicit Room(boost::log::sources::logger roomLoggger);
};

#endif