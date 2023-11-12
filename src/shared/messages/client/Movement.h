#ifndef BOOMBERMAN_MOVEMENT_H
#define BOOMBERMAN_MOVEMENT_H

#include "../../Message.h"

#define MOVEMENT_SIZE (2 * sizeof(float))

// Movement tells the server the new position of the player
class Movement : public Message {
public:
    float x;
    float y;

    [[nodiscard]] std::string name() const override;

    [[nodiscard]] MessageType type() const override;

    explicit Movement(float newX, float newY);
};

#endif