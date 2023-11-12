#include "LeaveGame.h"

std::string LeaveGame::name() const {
    return "Leave game";
}

MessageType LeaveGame::type() const {
    return LEAVEGAME;
}