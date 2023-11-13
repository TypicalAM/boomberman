#ifndef BOOMBERMAN_ILEAVE_H
#define BOOMBERMAN_ILEAVE_H

#include "../../Message.h"

#define ILEAVE_SIZE 0

// ILeave tells the server that we are leaving the game
class ILeave : public Message {
public:
    [[nodiscard]] std::string name() const override;

    [[nodiscard]] MessageType type() const override;
};

#endif