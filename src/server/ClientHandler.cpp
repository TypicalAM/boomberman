#include "ClientHandler.h"

#include <utility>
#include <iostream>

// TODO: Handle connection close
void ClientHandler::ReadLoop() {
    char buf[256];
    while (true) {
        auto msg = Channel::Receive(clientSock);
        if (!msg.has_value()) continue; // TODO: Is spinlocking here a good thing?
        std::cout << "Read a message of type: " << msg.value()->message_type() << std::endl;
        msgMtx->lock();
        msgQueue->push(std::move(msg.value()));
        msgMtx->unlock();
        std::cout << "The message was pushed into the queue" << std::endl;
    }
}

int ClientHandler::GetClient() const {
    return clientSock;
}

ClientHandler::ClientHandler(int fd, std::shared_ptr<std::queue<std::unique_ptr<GameMessage>>> queue,
                             std::shared_ptr<std::mutex> mtx) {
    clientSock = fd;
    msgQueue = std::move(queue);
    msgMtx = std::move(mtx);
}