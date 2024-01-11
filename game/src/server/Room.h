#ifndef BOOMBERMAN_ROOM_H
#define BOOMBERMAN_ROOM_H

#include "../shared/game/Bomb.h"
#include "../shared/game/Map.h"
#include "../shared/proto/messages.pb.h"
#include "entities/Primitives.h"
#include "entities/SPlayer.h"
#include <boost/log/sources/logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <memory>
#include <mutex>
#include <optional>
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

// the first is the number of bombs that should be exploded
// the second is the connections that should be deleted (releasing room
// assignments, connectiion objects)
typedef std::pair<int, std::vector<int>> PlayerDestructionInfo;

/**
 * @class Room
 * Contains logic connected to handling a room gamestate
 */
class Room {
public:
  Room(boost::log::sources::logger roomLoggger, int epollSock);
  int PlayerCount();
  bool CanJoin(const std::string &username);
  SPlayer *JoinPlayer(Connection *conn, const std::string &username);
  bool IsGameOver();
  std::vector<std::unique_ptr<SPlayer>> players;
  bool HandleMessage(std::unique_ptr<AuthoredMessage> msg);
  void NotifyExplosion();
  PlayerDestructionInfo DisconnectPlayers();

private:
  std::queue<Bomb> bombs;
  std::atomic<GameState> state = WAIT_FOR_START;
  std::mutex playerMtx;
  int clientCount = 0;
  boost::log::sources::logger logger;
  std::unique_ptr<Map> map;
  int epollSock;

  template <typename Function, typename... Args>
  void SendSpecific(SPlayer *player, Function &&builderFunc,
                    Args &&...builderArgs);

  template <typename Function, typename... Args>
  void SendExcept(SPlayer *player, Function &&builderFunc,
                  Args &&...builderArgs);

  template <typename Function, typename... Args>
  void SendBroadcast(Function &&builderFunc, Args &&...builderArgs);
};

#endif
