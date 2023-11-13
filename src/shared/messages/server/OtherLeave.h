#ifndef BOOMBERMAN_OTHERLEAVE_H
#define BOOMBERMAN_OTHERLEAVE_H

#include <utility>

#include "../../Message.h"

class OtherLeave : public Message {
public:
    std::string playerName;

    [[nodiscard]] std::string name() const override { return "Other Leave"; }

    [[nodiscard]] MessageType type() const override { return OTHERLEAVE; }

    size_t size() { return playerName.length() + 1; }

    explicit OtherLeave(std::string playerName) {
        this->playerName = std::move(playerName);
    }
};

#endif