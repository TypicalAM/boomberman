#include "TileOnFire.h"

TileOnFire::TileOnFire(int x, int y) {
    this->x=x;
    this->y=y;
    this->on_fire_since=GetTime();
    this->should_burn_for=1.0f;
}

int TileOnFire::shouldCalmDown() {
    return GetTime() - this->on_fire_since >= this->should_burn_for;
}
