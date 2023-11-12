#ifndef BOOMBERMAN_LEAVEGAME_H
#define BOOMBERMAN_LEAVEGAME_H

#include "../../Message.h"

#define LEAVEGAME_SIZE 0

// LeaveGame tells the server that we are leaving the game
class LeaveGame : public Message {
public:
    [[nodiscard]] std::string name() const override;

    [[nodiscard]] MessageType type() const override;
};

#endif