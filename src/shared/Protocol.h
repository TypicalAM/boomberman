#ifndef BOOMBERMAN_PROTOCOL_H
#define BOOMBERMAN_PROTOCOL_H

#include <optional>
#include "Message.h"

class Protocol {
    static std::optional<int> Encode(Message msg, char *buf[256]); // Encode into buffer, optionally return cb
    static std::optional<Message> Decode(char buf[256], int cb); // Decode from buffer, cb is the message length
};

#endif