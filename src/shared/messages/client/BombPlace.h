#ifndef BOOMBERMAN_BOMBPLACE_H
#define BOOMBERMAN_BOMBPLACE_H

#include <string>
#include "../../Protocol.h"

#define BOMBPLACE_SIZE (2 * sizeof(float))

// BombPlace tells the server that we are placing a bomb
class BombPlace : public Message {
public:
    float x;
    float y;

    [[nodiscard]] std::string name() const override;

    [[nodiscard]] MessageType type() const override;

    explicit BombPlace(float coordX, float coordY);
};

#endif