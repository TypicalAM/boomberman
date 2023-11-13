#include "IPlaceBomb.h"

std::string IPlaceBomb::name() const {
    return "Bomb place";
}

MessageType IPlaceBomb::type() const {
    return BOMBPLACE;
}

IPlaceBomb::IPlaceBomb(float coordX, float coordY) {
    x = coordX;
    y = coordY;
}