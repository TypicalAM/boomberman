#ifndef BOOMBERMAN_IMOVE_H
#define BOOMBERMAN_IMOVE_H

#include "../../Message.h"

// IMove tells the server the new position of the player
class IMove : public Message {
public:
    float x;
    float y;

    [[nodiscard]] std::string name() const override { return "I Move"; }

    [[nodiscard]] MessageType type() const override { return IMOVE; }

    static size_t size() { return 2 * sizeof(float); }

    explicit IMove(float newX, float newY) {
        x = newX;
        y = newY;
    }
};

#endif