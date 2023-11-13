#ifndef BOOMBERMAN_ILEAVE_H
#define BOOMBERMAN_ILEAVE_H

#include "../../Message.h"

// ILeave tells the server that we are leaving the game
class ILeave : public Message {
public:
    [[nodiscard]] std::string name() const override { return "I Leave Game"; }

    [[nodiscard]] MessageType type() const override { return ILEAVE; }

    static size_t size() { return 0; }
};

#endif