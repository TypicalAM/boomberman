#ifndef BOOMBERMAN_GAMEJOIN_H
#define BOOMBERMAN_GAMEJOIN_H

#include "../../Message.h"

#define GAMEJOIN_SIZE 1

enum Color {
    RED,
    GREEN,
    BLUE,
    YELLOW,
};

// Gamejoin tells the newly connected client the color of his character
class GameJoin : public Message {
public:
    Color color;

    [[nodiscard]] std::string name() const override;

    [[nodiscard]] MessageType type() const override;

    explicit GameJoin(Color color);
};

#endif