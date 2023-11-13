#ifndef BOOMBERMAN_IPLACEBOMB_H
#define BOOMBERMAN_IPLACEBOMB_H

#include <string>
#include "../../Protocol.h"

// IPlaceBomb tells the server that we are placing a bomb
class IPlaceBomb : public Message {
public:
    float x;
    float y;

    [[nodiscard]] std::string name() const override { return "I Place Bomb"; }

    [[nodiscard]] MessageType type() const override { return IPLACEBOMB; }

    static size_t size() { return 2 * sizeof(float); }

    explicit IPlaceBomb(float coordX, float coordY) {
        x = coordX;
        y = coordY;
    }
};

#endif