#ifndef BOOMBERMAN_GAMEWON_H
#define BOOMBERMAN_GAMEWON_H

#include "../../Message.h"

class GameWon : public Message {
public:
    std::string playerName;

    [[nodiscard]] std::string name() const override { return "Game won"; }

    [[nodiscard]] MessageType type() const override { return GAMEWON; }

    size_t size() { return playerName.length() + 1; }

    explicit GameWon(std::string playerName) {
        this->playerName = std::move(playerName);
    }
};

#endif