#ifndef BOOMBERMAN_MESSAGE_H
#define BOOMBERMAN_MESSAGE_H

#include <string>

enum MessageType {
    NONE, // TODO: This shouldn't exist
    GAMEJOIN,
};

class Message {
public:
    [[nodiscard]] virtual std::string name() const { return "No"; };

    [[nodiscard]] virtual MessageType type() const { return NONE; };

    virtual ~Message() = default;
};

#endif
