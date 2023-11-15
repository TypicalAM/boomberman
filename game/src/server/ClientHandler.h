#ifndef BOOMBERMAN_CLIENTHANDLER_H
#define BOOMBERMAN_CLIENTHANDLER_H

#include <queue>
#include <memory>
#include <mutex>
#include "../shared/Channel.h"

class ClientHandler {
private:
    std::shared_ptr<std::queue<std::unique_ptr<GameMessage>>> msgQueue;
    std::shared_ptr<std::mutex> msgMtx;
    int clientSock;

public:
    [[noreturn]] void ReadLoop();

    int GetClient() const;

    ClientHandler(int fd, std::shared_ptr<std::queue<std::unique_ptr<GameMessage>>> queue,
                  std::shared_ptr<std::mutex> mtx);
};

#endif