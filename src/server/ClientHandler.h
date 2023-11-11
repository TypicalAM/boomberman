#ifndef BOOMBERMAN_CLIENTHANDLER_H
#define BOOMBERMAN_CLIENTHANDLER_H

#include <queue>
#include <memory>
#include <mutex>
#include "../shared/Message.h"

class ClientHandler {
private:
    int clientSock;
    std::shared_ptr<std::queue<std::unique_ptr<Message>>> msgQueue;
    std::shared_ptr<std::mutex> msgMtx;

public:
    void Write(Message *msg);

    void ReadLoop();

    ClientHandler(int fd, std::shared_ptr<std::queue<std::unique_ptr<Message>>> queue, std::shared_ptr<std::mutex> mtx);
};

#endif