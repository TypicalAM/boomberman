#include "SPlayer.h"
#include "../../shared/Util.h"

#include <utility>

SPlayer::SPlayer(Connection *conn, std::string username, PlayerColor color) {
    this->conn = conn;
    this->username = std::move(username);
    this->color = color;
    this->livesRemaining = STARTER_LIVES;
    this->immunityEndTimestamp = Util::TimestampMillis();

    switch (color) {
        case PLAYER_RED:
            this->coords = Coords{1.0f, 1.0f};
            break;
        case PLAYER_GREEN:
            this->coords = Coords{15.0f, 1.0f};
            break;
        case PLAYER_BLUE:
            this->coords = Coords{1.0f, 9.0f};
            break;
        case PLAYER_YELLOW:
            this->coords = Coords{15.0f, 9.0f};
            break;
        default:
            throw std::runtime_error("unhandled color starting position");
    }
}