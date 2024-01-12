#ifndef BOOMBERMAN_SPLAYER_H
#define BOOMBERMAN_SPLAYER_H

#include "../../shared/Util.h"
#include "../../shared/msg/Connection.h"
#include "../../shared/proto/messages.pb.h"
#include "Primitives.h"
#include <array>
#include <string>

#define STARTER_LIVES 3
#define IMMUNITY_TIME_MILLIS 500
#define PLAYER_SUS_TRESHHOLD_MILLIS 150

struct MovementEntry {
  Timestamp ts;
  Coords coords;
};

class Player {
private:
  Coords coords{};
  std::array<MovementEntry, 5> moveHistory{};
  int step;

public:
  Connection *conn;
  int livesRemaining;
  std::string username;
  PlayerColor color;
  Timestamp immunityEndTimestamp = 0;
  bool marked_for_disconnect = false;

  Coords GetCoords();
  std::optional<Coords> MoveCheckSus(int x, int y);
  Player(Connection *conn, std::string username, PlayerColor color);
};

#endif
