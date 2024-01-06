#include "Map.h"

Map::Map(int size, int width, int height) {
  this->size = size;
  this->offset = int((this->size * 4) / 3);
  this->start_x = int(width / 2 - (cols / 2) * offset);
  this->start_y = int(height / 2 - (rows / 2) * offset);
}
int Map::getSquareState(int x, int y) { return this->map[x][y]; }
void Map::setSquareState(int x, int y, int state) { this->map[x][y] = state; }
