#ifndef BOOMBERMAN_GOTHIT_H
#define BOOMBERMAN_GOTHIT_H

#include <utility>

#include "../../Message.h"

// GotHit is sent to the player when a bomb explodes
class GotHit : public Message {
public:
    std::string playerName;
    int livesRemaining;

    [[nodiscard]] std::string name() const override { return "Got Hit"; }

    [[nodiscard]] MessageType type() const override { return GOTHIT; }

    size_t size() { return playerName.length() + 1 + sizeof(livesRemaining); }

    explicit GotHit(std::string name, int livesRemaining) {
        this->playerName = std::move(name);
        this->livesRemaining = livesRemaining;
    }
};

#endif