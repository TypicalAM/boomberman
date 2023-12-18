#ifndef BOOMBERMAN_SPLAYER_H
#define BOOMBERMAN_SPLAYER_H

#include <string>
#include <utility>
#include "Primitives.h"
#include "../../shared/proto/messages.pb.h"
#include "../../shared/msg/Connection.h"

#define STARTER_LIVES 3

class SPlayer {
public:
    std::unique_ptr<Connection> conn;
    int livesRemaining;
    Coords coords{};
    std::string username;
    PlayerColor color;

    SPlayer(std::unique_ptr<Connection> conn, std::string username, PlayerColor color);
};

#endif