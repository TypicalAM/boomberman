#ifndef BOOMBERMAN_GAMEJOIN_H
#define BOOMBERMAN_GAMEJOIN_H

#include <utility>

#include "../../Message.h"

enum Color {
    RED,
    GREEN,
    BLUE,
    YELLOW,
};

// Gamejoin tells the newly connected client some info
class GameJoin : public Message {
public:
    Color color;
    bool you;
    std::string playerName;

    [[nodiscard]] std::string name() const override { return "Game Join"; }

    [[nodiscard]] MessageType type() const override { return GAMEJOIN; }

    size_t size() { return playerName.length() + 3; } // 1 is for null terminator

    GameJoin(Color color, std::string name, bool you) {
        this->color = color;
        this->playerName = std::move(name);
        this->you = you;
    }
};

#endif