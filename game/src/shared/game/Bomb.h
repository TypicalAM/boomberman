#ifndef BOOMBERMAN_BOMB_H
#define BOOMBERMAN_BOMB_H

#include "TileOnFire.h"
#include "Map.h"
#include <vector>
#include "../Util.h"

#define FUSE_TIME_SEC 3

class Bomb{
public:
    int pos_x, pos_y;
    int explosion, size;
    double long plant_time, animation_start, ttl;
    int state;
    bool is_atomic;
    Bomb(int pos_x, int pos_y, int explosion, int size, long double timestamp, float ttl, bool is_atomic);
    [[nodiscard]] bool ShouldExplode() const;
    void animate();
    std::vector<TileOnFire> boom(Map *map);
    static int CreateBombTimerfd();
};

#endif //BOOMBERMAN_BOMB_H
