#ifndef BOOMBERMAN_SERVER_H
#define BOOMBERMAN_SERVER_H

#include <unordered_map>
#include <mutex>
#include <optional>
#include "Room.h"

class Server {
private:
    std::unordered_map<std::string, std::shared_ptr<Room>> rooms;
    std::vector<int> lobbySockets;
    std::mutex roomsMtx;
    int srvSock;
    int epollSock;
    boost::log::sources::logger logger;

    void handleClientMessage(Connection conn, std::unique_ptr<GameMessage> msg);

public:
    [[noreturn]] void Run();

    static boost::log::sources::logger createNamedLogger(const std::string &name);

    explicit Server(int port);
};

#endif