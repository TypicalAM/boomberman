#ifndef BOOMBERMAN_BOOMBERMAN_H
#define BOOMBERMAN_BOOMBERMAN_H

#include "Map.h"
#include <memory>

class Boomberman{
private:
    int* position;
    int health;
public:
    int id;
    Boomberman(int id, int start_x, int start_y, int health);
    std::unique_ptr<int[]> getBoombermanPos();
    void setBoombermanPos(int new_x, int new_y);
    void move(Map *map,std::shared_ptr<int[]> curr_pos, int x, int y);
    void cleanUp();
};

#endif //BOOMBERMAN_BOOMBERMAN_H
