#ifndef BOOMBERMAN_MAP_H
#define BOOMBERMAN_MAP_H

#include <vector>
#include <raylib.h>
#include "../client/Client.h"

#define MAP_WIDTH 17
#define MAP_HEIGHT 11

class Map {
private:
    int map[MAP_WIDTH][MAP_HEIGHT]={
                     {1,1,1,1,1,1,1,1,1,1,1},
                     {1,0,0,2,2,2,2,2,0,0,1},
                     {1,0,1,2,1,2,1,2,1,0,1},
                     {1,2,2,2,2,2,2,2,2,2,1},
                     {1,2,1,2,1,2,1,2,1,2,1},
                     {1,2,2,2,2,2,2,2,2,2,1},
                     {1,2,1,2,1,2,1,2,1,2,1},
                     {1,2,2,2,2,2,2,2,2,2,1},
                     {1,2,1,2,1,2,1,2,1,2,1},
                     {1,2,2,2,2,2,2,2,2,2,1},
                     {1,2,1,2,1,2,1,2,1,2,1},
                     {1,2,2,2,2,2,2,2,2,2,1},
                     {1,2,1,2,1,2,1,2,1,2,1},
                     {1,2,2,2,2,2,2,2,2,2,1},
                     {1,0,1,2,1,2,1,2,1,0,1},
                     {1,0,0,2,2,2,2,2,0,0,1},
                     {1,1,1,1,1,1,1,1,1,1,1}};

    int cols = MAP_WIDTH;
    int rows= MAP_HEIGHT;

public:
    int offset{}, start_x{}, start_y{};
    int size;
    explicit Map(Client* client, int size);
    int getSquareState(int x, int y);
    void setSquareState(int x, int y, int state);
    void drawMap(Client *client);
    void debug();
};
#endif //BOOMBERMAN_MAP_H
