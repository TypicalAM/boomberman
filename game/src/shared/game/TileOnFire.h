#ifndef BOOMBERMAN_TILEONFIRE_H
#define BOOMBERMAN_TILEONFIRE_H

class TileOnFire {
public:
    int x, y;
    double on_fire_since, should_burn_for;
    int shouldCalmDown() const;

    TileOnFire(int x, int y);
};


#endif
