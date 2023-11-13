#ifndef BOOMBERMAN_OTHERMOVE_H
#define BOOMBERMAN_OTHERMOVE_H

#include "../../Message.h"

class OtherMove : public Message {
public:
    float x;
    float y;
    std::string playerName;

    [[nodiscard]] std::string name() const override { return "Other Move"; }

    [[nodiscard]] MessageType type() const override { return OTHERMOVE; }

    size_t size() { return playerName.length() + 1 + 2 * sizeof(float); }

    OtherMove(std::string playerName, float x, float y) {
        this->playerName = playerName;
        this->x = x;
        this->y = y;
    }
};

#endif