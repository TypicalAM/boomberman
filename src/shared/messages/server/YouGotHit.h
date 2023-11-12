#ifndef BOOMBERMAN_YOUGOTHIT_H
#define BOOMBERMAN_YOUGOTHIT_H

#include "../../Message.h"

#define YOUGOTHIT_SIZE 0

// YouGotHit is sent to the player when the bomb explodes in their face
class YouGotHit : public Message {
    [[nodiscard]] std::string name() const override;

    [[nodiscard]] MessageType type() const override;
};

#endif