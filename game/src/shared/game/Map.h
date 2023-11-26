#ifndef BOOMBERMAN_MAP_H
#define BOOMBERMAN_MAP_H

#include <vector>

#define MAP_WIDTH 17
#define MAP_HEIGHT 11

#define NOTHIN 0
#define HARD_WALL 1
#define SOFT_WALL 2

class Map {
private:
    int map[MAP_WIDTH][MAP_HEIGHT] = {
            {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
            {1, 0, 0, 2, 2, 2, 2, 2, 0, 0, 1},
            {1, 0, 1, 2, 1, 2, 1, 2, 1, 0, 1},
            {1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1},
            {1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1},
            {1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1},
            {1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1},
            {1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1},
            {1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1},
            {1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1},
            {1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1},
            {1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1},
            {1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1},
            {1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1},
            {1, 0, 1, 2, 1, 2, 1, 2, 1, 0, 1},
            {1, 0, 0, 2, 2, 2, 2, 2, 0, 0, 1},
            {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}};


public:
    int cols = MAP_WIDTH;
    int rows = MAP_HEIGHT;
    int offset{}, start_x{}, start_y{};
    int size;

    explicit Map(int size, int width, int height);

    int getSquareState(int x, int y);

    void setSquareState(int x, int y, int state);
};

#endif //BOOMBERMAN_MAP_H
