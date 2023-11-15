#ifndef BOOMBERMAN_CHANNEL_H
#define BOOMBERMAN_CHANNEL_H

#include <optional>
#include <memory>
#include "messages.pb.h"

// Send and receive messages from sockets
namespace Channel {
    std::optional<int>
    Send(int sock, std::unique_ptr<GameMessage> msg); // Send a message and if successful get bytes sent

    std::optional<std::unique_ptr<GameMessage>>
    Receive(int sock); // Block until a message is received or socket gets closed
};

#endif