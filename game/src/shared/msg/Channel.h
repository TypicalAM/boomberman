#ifndef BOOMBERMAN_CHANNEL_H
#define BOOMBERMAN_CHANNEL_H

#include <optional>
#include <memory>
#include <queue>
#include "../proto/messages.pb.h"

// Send and receive messages from sockets
class Channel {
private:
    std::queue<std::unique_ptr<GameMessage>> inboundQueue;

public:
    int sock;
    std::optional<int> Send(std::unique_ptr<GameMessage> msg); // Send a message and if successful get bytes sent
    std::optional<std::unique_ptr<GameMessage>> Receive(); // Block until a message is received or socket gets closed

    explicit Channel(int sock);
};

#endif