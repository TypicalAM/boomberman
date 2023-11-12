#include <iostream>
#include <memory>
#include <cstring>
#include "Protocol.h"
#include "messages/server/GameJoin.h"
#include "messages/client/Movement.h"
#include "messages/client/BombPlace.h"
#include "messages/client/LeaveGame.h"
#include "messages/server/YouGotHit.h"


std::optional<int> Protocol::Encode(Message *msg, char buf[256]) {
    switch (msg->type()) {
        case NONE:
            return std::nullopt;

        case GAMEJOIN: {
            auto joinMsg = dynamic_cast<GameJoin *>(msg);
            buf[0] = GAMEJOIN;
            buf[1] = joinMsg->color;
            return GAMEJOIN_SIZE + 1;
        }

        case MOVEMENT: {
            auto movementMsg = dynamic_cast<Movement *>(msg);
            buf[0] = MOVEMENT;
            std::memcpy(buf, &movementMsg->x + 1, sizeof(float));
            std::memcpy(buf, &movementMsg->y + 1 + sizeof(float), sizeof(float));
            return MOVEMENT_SIZE + 1;
        }

        case BOMBPLACE: {
            auto bombplaceMsg = dynamic_cast<BombPlace * >(msg);
            buf[0] = BOMBPLACE;
            std::memcpy(buf, &bombplaceMsg->x + 1, sizeof(float));
            std::memcpy(buf, &bombplaceMsg->y + 1 + sizeof(float), sizeof(float));
            return BOMBPLACE_SIZE + 1;
        }

        case LEAVEGAME: {
            buf[0] = LEAVEGAME;
            return LEAVEGAME_SIZE + 1;
        }

        case YOUGOTHIT: {
            buf[0] = YOUGOTHIT;
            return YOUGOTHIT_SIZE + 1;
        }
    }

    return std::nullopt;
}

std::optional<std::unique_ptr<Message>> Protocol::Decode(char buf[256], int cb) {
    switch (buf[0]) {
        case NONE:
            return std::nullopt;

        case GAMEJOIN:
            return std::make_unique<GameJoin>(static_cast<Color>(buf[1]));

        case MOVEMENT: {
            float x, y;
            std::memcpy(&x, &buf[1], sizeof(float));
            std::memcpy(&y, &buf[sizeof(float) + 1], sizeof(float));
            return std::make_unique<Movement>(x, y);
        }

        case BOMBPLACE: {
            float x, y;
            std::memcpy(&x, &buf[1], sizeof(float));
            std::memcpy(&y, &buf[sizeof(float) + 1], sizeof(float));
            return std::make_unique<BombPlace>(x, y);
        }

        case LEAVEGAME:
            return std::make_unique<LeaveGame>();

        case YOUGOTHIT:
            return std::make_unique<YouGotHit>();
    }

    return std::nullopt;
}
