#include "SPlayer.h"

SPlayer::SPlayer(int fd, std::string username, PlayerColor color) {
    this->sock = fd;
    this->username = std::move(username);
    this->color = color;
    this->livesRemaining = STARTER_LIVES;

    switch (color) {
        case PLAYER_RED: // TODO: Place players correctly
        case PLAYER_GREEN:
        case PLAYER_BLUE:
        case PLAYER_YELLOW:
            this->coords = Coords{1.0f, 1.0f};
            break;
        default:
            throw std::runtime_error("unhandled color starting position");
    }
}