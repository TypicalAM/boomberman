#ifndef BOOMBERMAN_ROOM_H
#define BOOMBERMAN_ROOM_H

#include <vector>
#include <mutex>
#include <memory>

#define MAX_PLAYERS 4

class Room {
private:
    std::string name;
    std::vector<int> clients;
    std::unique_ptr<std::mutex> clientMtx;
    int clientCount = 0;

public:
    void JoinPlayer(int fd);

    int AvailableSpace() const;

    void GameLoop();

    explicit Room(std::string name);
};

#endif