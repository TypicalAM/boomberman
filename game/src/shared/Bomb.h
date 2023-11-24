#ifndef BOOMBERMAN_BOMB_H
#define BOOMBERMAN_BOMB_H

#include "TileOnFire.h"
#include "Map.h"
#include <vector>

class Bomb{
public:
    int pos_x, pos_y;
    int explosion, size;
    double plant_time, animation_start, ttl;
    int state;
    bool should_explode=false;
    bool is_atomic;
    Bomb(int pos_x, int pos_y, int explosion, int size, float ttl, bool is_atomic);
    void animateOrBoom();
    std::vector<TileOnFire> boom(Map *map);
};

#endif //BOOMBERMAN_BOMB_H
