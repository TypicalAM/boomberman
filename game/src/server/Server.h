#ifndef BOOMBERMAN_SERVER_H
#define BOOMBERMAN_SERVER_H

#include <unordered_map>
#include <mutex>
#include <optional>
#include "Room.h"

#define MAX_EVENTS 25

struct PlayerInRoom {
    SPlayer *player;
    Room *room;
};

class Server {
private:
    std::unordered_map<std::string, std::shared_ptr<Room>> rooms;
    std::unordered_map<int, std::unique_ptr<Connection>> roomConns;
    std::unordered_map<int, PlayerInRoom> roomAssignments;
    int roomEpollSock;
    int bombEpollSock;

    std::vector<std::unique_ptr<Connection>> lobbyConns;
    std::vector<int> lobbySockets;
    std::mutex roomsMtx;
    int srvSock;
    int lobbyEpollSock;
    boost::log::sources::logger logger;

    void handleClientMessage(Connection *conn, std::unique_ptr<GameMessage> msg);

public:
    [[noreturn]] void RunLobby();

    static boost::log::sources::logger createNamedLogger(const std::string &name);

    explicit Server(int port);

    [[noreturn]] void RunRoom();

    [[noreturn]] void RunBombs();
};

#endif