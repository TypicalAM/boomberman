//
// Created by ignor on 11/21/23.
//

#ifndef BOOMBERMAN_WALL_H
#define BOOMBERMAN_WALL_H

class Wall{
private:
    int pos_x, pos_y;
    bool is_destructible;
public:
    Wall(int pos_x, int pos_y, bool is_descructible);
    int* getPosition() const;
    int isDescructible();
    void drawWall(int x, int y, int size);
};


#endif //BOOMBERMAN_WALL_H
