//
// Created by ignor on 11/21/23.
//

#ifndef BOOMBERMAN_ENTITYHANDLER_H
#define BOOMBERMAN_ENTITYHANDLER_H

#include "Boomberman.h"
#include "Bomb.h"
#include "Wall.h"

class EntityHandler{
public:
    std::vector<Boomberman> players;
    std::vector<Bomb> bombs;
    std::vector<Wall> walls;

    void placeWalls(Map* map);
    int destroyWall(Map* map, int x, int y);

    void drawWalls(Map* map);
    void drawPlayers(Map* map);
};

#endif //BOOMBERMAN_ENTITYHANDLER_H
