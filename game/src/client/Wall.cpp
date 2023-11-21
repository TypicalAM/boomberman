#include "Wall.h"
Wall::Wall(int pos_x, int pos_y, bool is_destructible) {
    this->pos_x=pos_x;
    this->pos_y=pos_y;
    this->is_destructible=is_destructible;
}

int* Wall::getPosition() const {
    int* positions = new int[2];
    positions[0]=this->pos_x;
    positions[1] = this->pos_y;

    return positions;
}