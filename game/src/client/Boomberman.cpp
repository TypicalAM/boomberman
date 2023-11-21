//
// Created by ignor on 11/21/23.
//

#include "Boomberman.h"
Boomberman::Boomberman(int id, int start_x, int start_y, int health) {
    this->position=new int[2];
    this->position[0]=start_x;
    this->position[1]=start_y;
    this->health=health;
    this->id=id;
}
int* Boomberman::getBoombermanPos() const{
    int* pos = new int[2];
    pos[0]=this->position[0];
    pos[1]=this->position[1];
    return pos;
}
void Boomberman::setBoombermanPos(int new_x, int new_y) {
    this->position[0]=new_x;
    this->position[1]=new_y;
}
void Boomberman::move(Map* map, int* curr_pos, int x, int y) {
    int new_x = curr_pos[0]+x;
    int new_y = curr_pos[1]+y;
    int is_wall = map->getSquareState(new_x,new_y);
    if(is_wall!=1) this->setBoombermanPos(new_x, new_y);
}

void Boomberman::shitYourself(Map *map) {
}