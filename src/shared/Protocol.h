#ifndef BOOMBERMAN_PROTOCOL_H
#define BOOMBERMAN_PROTOCOL_H

#include <optional>
#include "Message.h"
#include <memory>

class Protocol {
public:
    static std::optional<int> Encode(Message *msg, char buf[256]); // Encode into buffer, optionally return cb

    static std::optional<std::unique_ptr<Message>>
    Decode(char buf[256], int cb); // Decode from buffer, optionally return message
};

#endif