#include "GameJoin.h"

std::string GameJoin::name() const {
    return "Game join";
}

MessageType GameJoin::type() const {
    return GAMEJOIN;
}

GameJoin::GameJoin(Color color) {
    this->color = color;
}
