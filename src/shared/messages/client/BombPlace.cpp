#include "BombPlace.h"

std::string BombPlace::name() const {
    return "Bomb place";
}

MessageType BombPlace::type() const {
    return BOMBPLACE;
}

BombPlace::BombPlace(float coordX, float coordY) {
    x = coordX;
    y = coordY;
}