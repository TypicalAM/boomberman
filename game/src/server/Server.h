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

public:
    [[noreturn]] void Run();

    explicit Server(int port);

    void handleClientMessage(int sock, std::unique_ptr<GameMessage> msg);

    static boost::log::sources::logger createRoomLogger(const std::string& name);
};

#endif