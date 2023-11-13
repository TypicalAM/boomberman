#include "IMove.h"

std::string IMove::name() const {
    return "IMove";
}

MessageType IMove::type() const {
    return Message::type();
}

IMove::IMove(float newX, float newY) {
    x = newX;
    y = newY;
}
