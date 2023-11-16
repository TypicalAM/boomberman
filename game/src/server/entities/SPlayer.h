#ifndef BOOMBERMAN_SPLAYER_H
#define BOOMBERMAN_SPLAYER_H

#include <string>
#include <utility>
#include "Primitives.h"
#include "../../shared/messages.pb.h"

#define STARTER_LIVES 3

class SPlayer {
public:
    int sock;
    int livesRemaining;
    Coords coords;
    std::string username;
    Color color;

    SPlayer(int fd, std::string username, Color color);
};

#endif