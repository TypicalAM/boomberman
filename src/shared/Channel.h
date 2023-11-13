#ifndef BOOMBERMAN_CHANNEL_H
#define BOOMBERMAN_CHANNEL_H

#include <optional>
#include <memory>
#include "messages.pb.h"

// Channel is used to send and receive messages from sockets
class Channel {
public:
    static std::optional<int>
    Send(int sock, std::unique_ptr<GameMessage> msg); // Send a message and if successful get bytes sent

    static std::optional<std::unique_ptr<GameMessage>>
    Receive(int sock); // Block until a message is received or socket gets closed
};

class Builder {
public:
    // Methods for building various messages
    static std::unique_ptr<GameMessage> IPlaceBomb(float x, float y);

    static std::unique_ptr<GameMessage> GameJoin(const std::string &name, Color color, bool you);

    static std::unique_ptr<GameMessage> GameWait(int32_t waitingFor);

    static std::unique_ptr<GameMessage> OtherBombPlace(int64_t timestamp, const std::string &name, float x, float y);

    static std::unique_ptr<GameMessage> GotHitMessage(const std::string &name, int32_t livesRemaining);

    static std::unique_ptr<GameMessage> OtherMove(const std::string &name, float x, float y);

    static std::unique_ptr<GameMessage> GameWon(const std::string &winner);
};

#endif