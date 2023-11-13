#ifndef BOOMBERMAN_OTHERBOMBPLACE_H
#define BOOMBERMAN_OTHERBOMBPLACE_H

#include <chrono>
#include <utility>
#include "../../Message.h"

class OtherBombPlace : public Message {
public:
    float x;
    float y;
    std::string playerName;

    [[nodiscard]] std::string name() const override { return "Other Bomb Place"; }

    [[nodiscard]] MessageType type() const override { return OTHERBOMBPLACE; }

    size_t size() { return playerName.length() + 1 + 2 * sizeof(float) + sizeof(long); }

    OtherBombPlace(std::string playerName, float x, float y, long timestamp) {
        this->playerName = std::move(playerName);
        this->x = x;
        this->y = y;
        this->timestamp = timestamp;
    }

public:
    long timestamp;
};

#endif