#ifndef BOOMBERMAN_GAMEWAIT_H
#define BOOMBERMAN_GAMEWAIT_H

#include "../../Message.h"

class GameWait : public Message {
public:
    int waitingFor;

    [[nodiscard]] std::string name() const override { return "Game wait"; }

    [[nodiscard]] MessageType type() const override { return GAMEWAIT; }

    static size_t size() { return sizeof(int); }

    explicit GameWait(int waitingFor) {
        this->waitingFor = waitingFor;
    }
};

#endif