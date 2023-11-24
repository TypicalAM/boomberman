#ifndef BOOMBERMAN_ENTITYHANDLER_H
#define BOOMBERMAN_ENTITYHANDLER_H

#include "Boomberman.h"
#include "../shared/Bomb.h"

class EntityHandler{
public:
    std::vector<Boomberman> players;
    std::vector<Bomb> bombs;
    std::vector<TileOnFire> theFloorIsLava;

    void placeBomb(int x, int y, int explostion, int size, int ttl, bool is_atomic);
    //static std::vector<TileOnFire> setOnFire(Bomb* bomb, Map* map);
    int explodeBomb(Bomb* bomb, Map* map);
    void tryExtinguish();

    void drawPlayers(Map* map);
    void drawBombs(Map* map);
    void drawFire(Map* map);
};

#endif
