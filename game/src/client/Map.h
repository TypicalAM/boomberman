#ifndef BOOMBERMAN_MAP_H
#define BOOMBERMAN_MAP_H

#include <vector>
#include "Client.h"
#include "Bomb.h"

class Map {
private:
    int map[MAP_WIDTH][MAP_HEIGHT]{};
    int cols = int(sizeof(map)/ sizeof(map[-1]));
    int rows= int(sizeof(map[-1])/ sizeof(int));
    std::vector<Bomb> bombs;
public:
    int offset{}, start_x{}, start_y{};
    int size;
    explicit Map(Client* client, int size);
    int* getColsNrows();
    int getSquareState(int x, int y);
    void setSquareState(int x, int y, int state);
    void drawMap(Client *client);
    void debug();
};
#endif //BOOMBERMAN_MAP_H
