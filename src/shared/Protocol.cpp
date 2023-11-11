#include "Protocol.h"
#include "messages/server/GameJoin.h"


std::optional<int> Protocol::Encode(Message msg, char buf[256]) {
    switch (msg.type()) {
        case NONE:
            return std::nullopt;

        case GAMEJOIN:
            auto joinMsg = dynamic_cast<GameJoin *>(&msg);
            buf[0] = GAMEJOIN;
            buf[1] = joinMsg->color;
            return GAMEJOIN_SIZE;
    }

    return std::nullopt;
}

std::optional<Message> Protocol::Decode(char buf[256], int cb) {
    switch (buf[0]) {
        case NONE:
            return std::nullopt;

        case GAMEJOIN:
            auto g = GameJoin();
            g.color = static_cast<Color>(buf[1]);
            return g;
    }

    return std::nullopt;
}
