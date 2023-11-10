#ifndef BOOMBERMAN_SEVER_H
#define BOOMBERMAN_SEVER_H

#include <unordered_map>
#include <mutex>
#include "Room.h"

class Server {
private:
    std::unordered_map<std::string, std::unique_ptr<Room>> rooms;
    std::mutex roomsMtx;
    int port;

public:
    void Run();

    explicit Server(int port);
};

#endif