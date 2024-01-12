#ifndef BOOMBERMAN_PLAYER_H
#define BOOMBERMAN_PLAYER_H

#include "../../shared/Util.h"
#include "../../shared/msg/Connection.h"
#include "../../shared/proto/messages.pb.h"
#include "Primitives.h"
#include <array>
#include <string>

#define STARTER_LIVES 3
#define IMMUNITY_TIME_MILLIS 500
#define PLAYER_SUS_TRESHHOLD_MILLIS 150

/**
 * @struct MovementEntry
 * Movement entries are used to track whether the user is moving too
 * fast
 */
struct MovementEntry {
  Timestamp ts;
  Coords coords;
};

/**
 * @class Player
 * Simple class representing a server-side Player
 */
class Player {
public:
  /**
   * A constructor for the Player class
   * @param conn socket connection
   * @param username username
   * @param color character color
   */
  Player(Connection *conn, std::string username, PlayerColor color);

  /**
   * Get the user's coordinates
   * @return user's coords
   */
  Coords GetCoords();

  /*
   * Try to move the player and check if the moment is suspicious. When it is
   * suspicious the user is not moved
   * @param x new x position
   * @param y new y position
   * @return Optionally a coordinate this user should be thrown back to
   */
  std::optional<Coords> MoveCheckSus(int x, int y);

  Connection *conn;
  int livesRemaining;
  std::string username;
  PlayerColor color;
  Timestamp immunityEndTimestamp = 0;
  bool markedForDisconnect = false;

private:
  Coords coords{};
  std::array<MovementEntry, 5> moveHistory{};
  int step;
};

#endif
