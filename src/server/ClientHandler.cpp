#include "ClientHandler.h"
#include "../shared/Protocol.h"

#include <utility>
#include <iostream>
#include <unistd.h>

void ClientHandler::Write(Message msg) {
    char buf[256];
    auto length = Protocol::Encode(msg, buf);
    if (!length.has_value())
        throw std::runtime_error("failed to encode message for sending");

    std::cout << "Sending a message of " << length.value() << " bytes" << std::endl;
    int bytes_sent = write(clientSock, buf, length.value());
    if (bytes_sent == -1)
        throw std::runtime_error("can't send message");
    if (bytes_sent != length.value())
        throw std::runtime_error("bytes sent aren't bytes produced");

    std::cout << "Sent a message of " << bytes_sent << " bytes" << std::endl;
}

void ClientHandler::ReadLoop() {
    char buf[256];
    int bytes_read = read(clientSock, buf, 256);
    if (bytes_read == -1)
        throw std::runtime_error("can't read bytes");

    std::cout << "Read " << bytes_read << " bytes, trying to decode..." << std::endl;
    auto msg = Protocol::Decode(buf, bytes_read);
    if (!msg.has_value())
        throw std::runtime_error("failed to decode message");

    std::cout << "Decoded a message of type: " << msg.value().name() << ", putting into queue" << std::endl;
    msgMtx->lock();
    msgQueue->push(msg.value());
    msgMtx->unlock();
    std::cout << "Message pushed into queue" << std::endl;
}

ClientHandler::ClientHandler(int fd, std::shared_ptr<std::queue<Message>> queue, std::shared_ptr<std::mutex> mtx) {
    clientSock = fd;
    msgQueue = std::move(queue);
    msgMtx = std::move(mtx);
}
