#ifndef BOOMBERMAN_CONNECTION_H
#define BOOMBERMAN_CONNECTION_H

#include "../Util.h"
#include "../proto/messages.pb.h"
#include <cstdint>
#include <memory>
#include <optional>
#include <queue>

#define CONN_TIMEOUT_MILLIS 1500

/**
 * \namespace builder
 * \brief building messages one brick at a time
 */
namespace Builder {
/*
 * @struct Room
 * building a room representation for protobuf
 */
struct Room {
  std::string name;
  int32_t players;
  int32_t maxPlayers;
};

/*
 * @struct Player
 * building a player representation for protobuf
 */
struct Player {
  std::string username;
  PlayerColor color;
};
} // namespace Builder

/*
 * @class Connection
 * manages the tcp connection, doesn't support non-blocking sockets
 */
class Connection {
public:
  /*
   * Constructor for the connection
   * @param a connected socket
   */
  explicit Connection(int sock);

  /*
   * Receive a message
   * @return GameMessage from queue if there are messages in the queue (see @ref
   * HasMoreMessages), otherwise block on read from the socket connection. If
   * there is an error/timeout while reading return nothing
   */
  std::optional<std::unique_ptr<GameMessage>> Receive();

  /*
   * Tells if there are messages in the inbound queue, needs to be called after
   * every receive
   * @return True if there are messsages in the queue, False otherwise
   */
  bool HasMoreMessages();

  // NOTE: For specific message types and what they mean, check out the
  // messages.proto definitions in the root of the repository

  std::optional<int> SendError(std::string error);

  std::optional<int> SendGetRoomList();

  std::optional<int>
  SendJoinRoom(const std::string &name,
               std::optional<std::string> roomName = std::nullopt);

  std::optional<int> SendIPlaceBomb(float x, float y);

  std::optional<int> SendIMove(float x, float y);

  std::optional<int> SendILeave();

  std::optional<int> SendRoomList(const std::vector<Builder::Room> &rooms);

  std::optional<int>
  SendWelcomeToRoom(const std::vector<Builder::Player> &players);

  std::optional<int> SendGameJoin(Builder::Player player);

  std::optional<int> SendGameStart();

  std::optional<int> SendOtherBombPlace(std::string username,
                                        Timestamp timestamp, float x, float y);

  std::optional<int> SendGotHit(const std::string &username,
                                int32_t livesRemaining, Timestamp timestamp);

  std::optional<int> SendOtherMove(std::string name, float x, float y);

  std::optional<int> SendOtherLeave(const std::string &username);

  std::optional<int> SendGameWon(std::string winnerUsername);

  std::optional<int> SendMovementCorrection(float x, float y);

  int sock;

private:
  std::unique_ptr<GameMessage> msg;
  std::queue<std::unique_ptr<GameMessage>> inboundQueue;

  [[nodiscard]] std::optional<int> send();
};

#endif
