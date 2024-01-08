#ifndef BOOMBERMAN_ROOM_H
#define BOOMBERMAN_ROOM_H

#include "../shared/game/Bomb.h"
#include "../shared/game/Map.h"
#include "../shared/proto/messages.pb.h"
#include "entities/SPlayer.h"
#include <boost/log/sources/logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <memory>
#include <mutex>
#include <queue>
#include <vector>

#define MAX_PLAYERS 3
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

/**
 * @class Room
 * Contains logic connected to handling a room gamestate
 */
class Room {
public:
  Room(boost::log::sources::logger roomLoggger, int epollSock);
  int PlayerCount();
  void Disconnect(SPlayer *player);
  bool CanJoin(const std::string &username);
  SPlayer *JoinPlayer(Connection *conn, const std::string &username);
  bool IsGameOver();
  std::vector<std::unique_ptr<SPlayer>> players;
  bool HandleMessage(std::unique_ptr<AuthoredMessage> msg);
  void NotifyExplosion();

private:
  std::queue<Bomb> bombs;
  std::atomic<GameState> state = WAIT_FOR_START;
  std::mutex playerMtx;
  int clientCount = 0;
  boost::log::sources::logger logger;
  std::unique_ptr<Map> map;
  int epollSock;

  template <typename Function, typename... Args>
  void SendSpecific(Connection *conn, Function &&builderFunc,
                    Args &&...builderArgs);

  template <typename Function, typename... Args>
  void SendExcept(Connection *conn, Function &&builderFunc,
                  Args &&...builderArgs);

  template <typename Function, typename... Args>
  void SendBroadcast(Function &&builderFunc, Args &&...builderArgs);
};

#endif
