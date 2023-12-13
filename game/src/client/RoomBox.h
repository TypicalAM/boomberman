#ifndef BOOMBERMAN_ROOMBOX_H
#define BOOMBERMAN_ROOMBOX_H

#include <string>
#include "raylib.h"


class RoomBox {
public:
    Rectangle rect;
    std::string label;
    int players_in;

    RoomBox(float x, float y, float width, float height, std::string label, int players_in);
};

#endif //BOOMBERMAN_ROOMBOX_H
