#ifndef BOOMBERMAN_ROOM_H
#define BOOMBERMAN_ROOM_H

#include <vector>
#include <mutex>
#include <memory>

#define MAX_PLAYERS 2

class Room {
private:
    std::string name;
    std::vector<int> clients;
    std::mutex clientMtx;
    int clientCount = 0;

public:
    void JoinPlayer(int fd);

    bool CanJoin();

    void GameLoop();

    std::string GetName();

    explicit Room(std::string name);
};

#endif