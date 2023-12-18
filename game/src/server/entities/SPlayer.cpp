#include "SPlayer.h"

#include <utility>

SPlayer::SPlayer(std::unique_ptr<Connection> conn, std::string username, PlayerColor color) : conn(std::move(conn)) {
    this->conn = std::move(conn);
    this->username = std::move(username);
    this->color = color;
    this->livesRemaining = STARTER_LIVES;

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