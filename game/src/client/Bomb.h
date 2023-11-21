//
// Created by ignor on 11/21/23.
//

#ifndef BOOMBERMAN_BOMB_H
#define BOOMBERMAN_BOMB_H

#include <raylib.h>

class Bomb{
public:
    int pos_x, pos_y;
    int explosion, size;
    double plant_time, animation_start, ttl;
    int state;
    bool should_explode=false;
    Bomb(int pos_x, int pos_y, int explosion, int size, float ttl);
    void animateOrBoom();
};

#endif //BOOMBERMAN_BOMB_H
