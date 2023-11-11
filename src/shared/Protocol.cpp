#include <iostream>
#include <memory>
#include "Protocol.h"
#include "messages/server/GameJoin.h"


std::optional<int> Protocol::Encode(Message *msg, char buf[256]) {
    switch (msg->type()) {
        case NONE:
            return std::nullopt;

        case GAMEJOIN:
            auto joinMsg = dynamic_cast<GameJoin *>(msg);
            buf[0] = GAMEJOIN;
            buf[1] = joinMsg->color;
            return GAMEJOIN_SIZE + 1;
    }

    return std::nullopt;
}

std::optional<std::unique_ptr<Message>> Protocol::Decode(char buf[256], int cb) {
    switch (buf[0]) {
        case NONE:
            return std::nullopt;

        case GAMEJOIN:
            std::cout << "kurwa: returning a gamejoin" << std::endl;
            return std::make_unique<GameJoin>(static_cast<Color>(buf[1]));
    }

    return std::nullopt;
}
