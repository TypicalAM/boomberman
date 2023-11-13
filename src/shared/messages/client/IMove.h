#ifndef BOOMBERMAN_IMOVE_H
#define BOOMBERMAN_IMOVE_H

#include "../../Message.h"

#define IMOVE_SIZE (2 * sizeof(float))

// IMove tells the server the new position of the player
class IMove : public Message {
public:
    float x;
    float y;

    [[nodiscard]] std::string name() const override;

    [[nodiscard]] MessageType type() const override;

    explicit IMove(float newX, float newY);
};

#endif