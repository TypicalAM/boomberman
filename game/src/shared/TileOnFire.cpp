#include "TileOnFire.h"

TileOnFire::TileOnFire(int x, int y) {
    this->x=x;
    this->y=y;
    this->on_fire_since=GetTime();
    this->should_burn_for=0.3f;
}

int TileOnFire::shouldCalmDown() const {
    return GetTime() - this->on_fire_since >= this->should_burn_for;
}
