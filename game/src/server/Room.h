#ifndef BOOMBERMAN_ROOM_H
#define BOOMBERMAN_ROOM_H

#include "../shared/game/Bomb.h"
#include "../shared/game/Map.h"
#include "../shared/proto/messages.pb.h"
#include "entities/Player.h"
#include <boost/log/sources/logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <vector>

#define MAX_PLAYERS 3
#define LOG BOOST_LOG(this->logger)

/**
 * Game state machine enum
 */
enum GameState {
  WAIT_FOR_START,
  PLAY,
  WAIT_FOR_END,
  GAME_OVER,
};

/**
 * @struct AuthoredMessage
 * Makes a message easily trackable
 */
struct AuthoredMessage {
  std::unique_ptr<GameMessage> payload;
  Player *author;
};

// pair.first is the number of bombs that should be exploded
// pair.second is the connections that should be deleted
typedef std::pair<int, std::vector<int>> PlayerDestructionInfo;

/**
 * @class Room
 * Contains logic connected to handling a room's state
 */
class Room {
public:
  /**
   * Constructor for the Room class
   * @param roomLogger named logger for the room
   * @param epollSock the server epoll socket used for this room's messages
   */
  Room(boost::log::sources::logger roomLoggger, int epollSock);

  /**
   * The number of players in the room
   * @return number of players in the room
   */
  int PlayerCount();

  /**
   * Decides whether a user can join a room and gives a reason if not
   * @return Optionally get a reason for rejection, for example "Username
   * already exists"
   */
  std::optional<std::string> CanJoin(const std::string &username);

  /*
   * Joins a player to the room using his username, the returned @ref Player
   * object is managed by this room
   * @param conn the player's connection
   * @param username the player's username
   * @return pointer to a room-allocated player (can fail in rare cases)
   */
  Player *JoinPlayer(Connection *conn, const std::string &username);

  /*
   * Is the game over?
   * @return True if the game is over, false otherwise
   */
  bool IsGameOver();

  /*
   * Handle the message of a user and respond accordingly, notify the upper
   * layers when we want a bomb to be placed
   * @param msg message to be processed
   * @return True if the server should create a bomb timer, False otherwise
   */
  bool HandleMessage(std::unique_ptr<AuthoredMessage> msg);

  /*
   * Notify the room that an explosion should be conducted, usually called by a
   * timer epoll thread
   */
  void NotifyExplosion();

  /*
   * Disconnect players which should be disconnected and clean up after them. If
   * atomic bombs are to be placed they get added into the destruction info.
   * @return info about the players which should be destroyed and how many bombs
   * are to be placed. Look @ref PlayerDestructionInfo
   */
  PlayerDestructionInfo DisconnectPlayers();

  std::vector<std::unique_ptr<Player>> players;

private:
  std::queue<Bomb> bombs;
  std::atomic<GameState> state = WAIT_FOR_START;
  std::mutex playerMtx;
  int clientCount = 0;
  boost::log::sources::logger logger;
  std::unique_ptr<Map> map;
  int epollSock;

  template <typename Function, typename... Args>
  void sendSpecific(Player *player, Function &&builderFunc,
                    Args &&...builderArgs);

  template <typename Function, typename... Args>
  void sendExcept(Player *player, Function &&builderFunc,
                  Args &&...builderArgs);

  template <typename Function, typename... Args>
  void sendBroadcast(Function &&builderFunc, Args &&...builderArgs);
};

#endif
