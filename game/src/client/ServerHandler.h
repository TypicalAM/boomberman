#ifndef BOOMBERMAN_SERVERHANDLER_H
#define BOOMBERMAN_SERVERHANDLER_H

#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/poll.h>
#include <error.h>

#include "Client.h"
#include "EntityHandler.h"
#include "../shared/msg/Connection.h"
#include "../shared/msg/Builder.h"
#include "raylib.h"

class ServerHandler {
private:
    int start_x{}, start_y{};
    pollfd polling[1]{};
    Color start_color{};
    std::unique_ptr<GameMessage> msg;
public:
    Connection conn;
    ServerHandler();
    void connect2Server(const char* ip, int port) const;
    static std::string selectUsername(float width, float height);
    void menu(float width, float height);
    void setPlayerParams(const GamePlayer& player);
    void wait4Game(EntityHandler &eh, float width, float height);
    void addPlayer(const GamePlayer& player, EntityHandler &eh);
    void joinRoom(EntityHandler &eh);
    void listRooms(float width, float height);

    [[noreturn]] void receiveLoop(EntityHandler &eh);
    static std::vector<Boomberman>::iterator findPlayer(EntityHandler &eh, const std::string& username);
};

#endif //BOOMBERMAN_SERVERHANDLER_H