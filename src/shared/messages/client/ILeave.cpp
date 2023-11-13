#include "ILeave.h"

std::string ILeave::name() const {
    return "Leave game";
}

MessageType ILeave::type() const {
    return LEAVEGAME;
}