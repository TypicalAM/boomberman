#ifndef BOOMBERMAN_SERVERHANDLER_H
#define BOOMBERMAN_SERVERHANDLER_H

#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "EntityHandler.h"
#include "../shared/msg/Channel.h"
#include "../shared/msg/Builder.h"
#include "raylib.h"

class ServerHandler {
private:
    int sock, start_x, start_y;
    Color start_color;
    std::unique_ptr<GameMessage> msg;
public:
    void connect2Server(const char* ip, int port);
    void getRoomList(const char* player_name);
    void setPlayerParams(const GamePlayer& player);
    void wait4Game(EntityHandler &eh);
    void addPlayer(GamePlayer player, EntityHandler &eh);
    void joinRoom(EntityHandler &eh);
    void otherJoinRoom(EntityHandler &eh);
};

#endif //BOOMBERMAN_SERVERHANDLER_H