#ifndef BOOMBERMAN_SERVER_H
#define BOOMBERMAN_SERVER_H

#include <unordered_map>
#include <mutex>
#include <optional>
#include "Room.h"

class Server {
private:
    std::unordered_map<std::string, std::shared_ptr<Room>> rooms;
    std::mutex roomsMtx;
    int port;

    std::optional<int> setup();

public:
    [[noreturn]] void Run();

    explicit Server(int port);
};

#endif