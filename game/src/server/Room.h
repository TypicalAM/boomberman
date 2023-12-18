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
    SPlayer *author;
};

class Room {
private:
    std::queue<Bomb> bombs;
    std::atomic<GameState> state = WAIT_FOR_START;
    Timestamp lastGameWaitMessage;

    std::mutex playerMtx;
    int clientCount = 0;

    boost::log::sources::logger logger;

    std::unique_ptr<Map> map;

    void SendSpecific(Connection *conn, std::unique_ptr<GameMessage> msg);

    template<typename Function, typename ...Args>
    void SendExcept(Connection *conn, Function &&builderFunc, Args &&... builderArgs);

    template<typename Function, typename ...Args>
    void SendBroadcast(Function &&builderFunc, Args &&... builderArgs);

public:
    int Players();

    bool CanJoin(const std::string &username);

    SPlayer *JoinPlayer(Connection *conn, const std::string &username);

    bool IsGameOver();

    explicit Room(boost::log::sources::logger roomLoggger);

    std::vector<std::unique_ptr<SPlayer>> players;

    bool HandleMessage(std::unique_ptr<AuthoredMessage> msg); // return true is if bomb has been placed

    void PlaceSuperBomb(SPlayer *player);

    void ExplodeBomb();
};

#endif