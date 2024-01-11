#ifndef BOOMBERMAN_SPLAYER_H
#define BOOMBERMAN_SPLAYER_H

#include "../../shared/Util.h"
#include "../../shared/msg/Connection.h"
#include "../../shared/proto/messages.pb.h"
#include "Primitives.h"
#include <string>
#include <utility>

#define STARTER_LIVES 3
#define IMMUNITY_TIME_MILLIS 500

class SPlayer {
public:
  Connection *conn;
  int livesRemaining;
  Coords coords{};
  std::string username;
  PlayerColor color;
  Timestamp immunityEndTimestamp;
  bool marked_for_disconnect = false;

  SPlayer(Connection *conn, std::string username, PlayerColor color);
};

#endif
