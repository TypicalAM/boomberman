#ifndef BOOMBERMAN_IPLACEBOMB_H
#define BOOMBERMAN_IPLACEBOMB_H

#include <string>
#include "../../Protocol.h"

#define IPLACEBOMB_SIZE (2 * sizeof(float))

// IPlaceBomb tells the server that we are placing a bomb
class IPlaceBomb : public Message {
public:
    float x;
    float y;

    [[nodiscard]] std::string name() const override;

    [[nodiscard]] MessageType type() const override;

    explicit IPlaceBomb(float coordX, float coordY);
};

#endif