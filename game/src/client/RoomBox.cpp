#include "RoomBox.h"

RoomBox::RoomBox(float x, float y, float width, float height, std::string label,
                 int players_in) {
  this->rect = {x, y, width, height};
  this->label = label;
  this->players_in = players_in;
}
