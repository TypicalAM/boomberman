//
// Created by ignor on 11/21/23.
//

#ifndef BOOMBERMAN_BOOMBERMAN_H
#define BOOMBERMAN_BOOMBERMAN_H

#include "Map.h"

class Boomberman{
private:
    int* position;
    int health;
public:
    int id;
    Boomberman(int id, int start_x, int start_y, int health);
    [[nodiscard]] int* getBoombermanPos() const;
    void setBoombermanPos(int new_x, int new_y);
    void move(Map *map, const int* current_position, int x, int y);
};

#endif //BOOMBERMAN_BOOMBERMAN_H
