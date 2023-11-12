#include "Movement.h"

std::string Movement::name() const {
    return "Movement";
}

MessageType Movement::type() const {
    return Message::type();
}

Movement::Movement(float newX, float newY) {
    x = newX;
    y = newY;
}
