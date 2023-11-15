#include "SPlayer.h"

SPlayer::SPlayer(int fd, std::string username, Color color) {
    this->sock = fd;
    this->coords.x = 0.0F;
    this->coords.y = 0.0F;
    this->username = std::move(username);
    this->color = color;
}