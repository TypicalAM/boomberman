#ifndef BOOMBERMAN_ENTITYHANDLER_H
#define BOOMBERMAN_ENTITYHANDLER_H

#include "Boomberman.h"
#include "Bomb.h"
#include "Wall.h"
#include "TileOnFire.h"

class EntityHandler{
public:
    std::vector<Boomberman> players;
    std::vector<Bomb> bombs;
    std::vector<Wall> walls;
    std::vector<TileOnFire> theFloorIsLava;

    void placeWalls(Map* map);
    void placeBomb(int x, int y, int explostion, int size, int ttl);
    void setOnFire(Bomb* bomb, Map* map);
    int destroyWall(Map* map, int x, int y);
    int explodeBomb(Bomb* bomb, Map* map);

    void drawWalls(Map* map);
    void drawPlayers(Map* map);
    void drawBombs(Map* map);
    void drawFire(Map* map);
};

#endif
