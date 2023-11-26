#include "SPlayer.h"

SPlayer::SPlayer(int fd, std::string username, Color color) {
    this->sock = fd;
    this->username = std::move(username);
    this->color = color;
    this->livesRemaining = STARTER_LIVES;

    switch (color) {
        case RED: // TODO: Place players correctly
        case GREEN:
        case BLUE:
        case YELLOW:
            this->coords = Coords{0.0f, 0.0f};
            break;
        default:
            throw std::runtime_error("unhandled color starting position");
    }
}