#include <memory>
#include <cstring>
#include <iostream>
#include "Protocol.h"
#include "messages/server/GameJoin.h"
#include "messages/client/IMove.h"
#include "messages/client/IPlaceBomb.h"
#include "messages/client/ILeave.h"
#include "messages/server/GotHit.h"
#include "messages/server/GameWait.h"
#include "messages/server/GameWon.h"
#include "messages/server/OtherBombPlace.h"
#include "messages/server/OtherMove.h"
#include "messages/server/OtherLeave.h"


std::optional<int> Protocol::Encode(Message *msg, char buf[256]) {
    switch (msg->type()) {
        case NONE:
            return std::nullopt;

        case IMOVE: {
            auto movementMsg = dynamic_cast<IMove *>(msg);
            buf[0] = IMOVE;
            std::memcpy(buf, &movementMsg->x + 1, sizeof(float));
            std::memcpy(buf, &movementMsg->y + 1 + sizeof(float), sizeof(float));
            return IMove::size() + 1;
        }

        case IPLACEBOMB: {
            auto IPlaceBombMsg = dynamic_cast<IPlaceBomb * >(msg);
            buf[0] = IPLACEBOMB;
            std::memcpy(buf, &IPlaceBombMsg->x + 1, sizeof(float));
            std::memcpy(buf, &IPlaceBombMsg->y + 1 + sizeof(float), sizeof(float));
            return IPlaceBomb::size() + 1;
        }

        case ILEAVE: {
            buf[0] = ILEAVE;
            return ILeave::size() + 1;
        }

        case GAMEJOIN: {
            auto joinMsg = dynamic_cast<GameJoin *>(msg);
            buf[0] = GAMEJOIN;
            buf[1] = joinMsg->color;
            buf[2] = joinMsg->you;
            std::cout << "Playername length: " << joinMsg->playerName.length() << std::endl;
            std::cout << "joinmsg size: " << joinMsg->size() + 1 << std::endl;
            std::memcpy(buf + 3, joinMsg->playerName.c_str(), joinMsg->playerName.length() + 1);
            return joinMsg->size() + 1;
        }

        case GAMEWAIT: {
            auto waitMsg = dynamic_cast<GameWait *>(msg);
            buf[0] = GAMEWAIT;
            std::memcpy(buf + 1, &waitMsg->waitingFor, sizeof(waitMsg->waitingFor));
            break;
        }

        case GAMEWON: {
            auto gameWonMsg = dynamic_cast<GameWon *>(msg);
            buf[0] = GAMEWON;
            std::memcpy(buf + 1, gameWonMsg->playerName.c_str(), gameWonMsg->playerName.length() + 1);
            break;
        }

        case GOTHIT: {
            auto gotHitMsg = dynamic_cast<GotHit *>(msg);
            buf[0] = GOTHIT;
            std::memcpy(buf + 1, &gotHitMsg->livesRemaining, sizeof(gotHitMsg->livesRemaining));
            std::memcpy(buf + sizeof(gotHitMsg->livesRemaining), gotHitMsg->playerName.c_str(),
                        gotHitMsg->playerName.length() + 1);
            return gotHitMsg->size() + 1;
        }

        case OTHERBOMBPLACE: {
            auto otherBombPlaceMsg = dynamic_cast<OtherBombPlace *>(msg);
            buf[0] = OTHERBOMBPLACE;
            std::memcpy(buf + 1, &otherBombPlaceMsg->timestamp, sizeof(otherBombPlaceMsg->timestamp));
            std::memcpy(buf + sizeof(otherBombPlaceMsg->timestamp) + 1, &otherBombPlaceMsg->x,
                        sizeof(otherBombPlaceMsg->x));
            std::memcpy(buf + sizeof(otherBombPlaceMsg->timestamp) + sizeof(otherBombPlaceMsg->x) + 1,
                        &otherBombPlaceMsg->y,
                        sizeof(otherBombPlaceMsg->y));
            std::memcpy(buf + sizeof(otherBombPlaceMsg->timestamp) + sizeof(otherBombPlaceMsg->x) +
                        sizeof(otherBombPlaceMsg->y) + 1,
                        otherBombPlaceMsg->playerName.c_str(), otherBombPlaceMsg->playerName.length() + 1);
            return otherBombPlaceMsg->size() + 1;
        }

        case OTHERMOVE: {
            auto otherMoveMsg = dynamic_cast<OtherMove *>(msg);
            buf[0] = OTHERMOVE;
            std::memcpy(buf + 1, &otherMoveMsg->x, sizeof(otherMoveMsg->x));
            std::memcpy(buf + sizeof(otherMoveMsg->x) + 1, &otherMoveMsg->y, sizeof(otherMoveMsg->y));
            std::memcpy(buf + sizeof(otherMoveMsg->x) + sizeof(otherMoveMsg->y) + 1, otherMoveMsg->playerName.c_str(),
                        otherMoveMsg->playerName.length() + 1);
            return otherMoveMsg->size() + 1;
        }

        case OTHERLEAVE: {
            auto otherLeaveMsg = dynamic_cast<OtherLeave *>(msg);
            buf[0] = OTHERLEAVE;
            std::memcpy(buf + 1, otherLeaveMsg->playerName.c_str(), otherLeaveMsg->playerName.length() + 1);
            return otherLeaveMsg->size();
        }

        default:
            return std::nullopt;
    }
}

std::optional<std::unique_ptr<Message>> Protocol::Decode(char buf[256], int cb) {
    switch (buf[0]) {
        case NONE:
            return std::nullopt;

        default:
            return std::nullopt;

        case IMOVE: {
            float x, y;
            std::memcpy(&x, &buf[1], sizeof(float));
            std::memcpy(&y, &buf[sizeof(float) + 1], sizeof(float));
            return std::make_unique<IMove>(x, y);
        }

        case IPLACEBOMB: {
            float x, y;
            std::memcpy(&x, &buf[1], sizeof(float));
            std::memcpy(&y, &buf[sizeof(float) + 1], sizeof(float));
            return std::make_unique<IPlaceBomb>(x, y);
        }

        case ILEAVE:
            return std::make_unique<ILeave>();
    }

    return std::nullopt;
}
