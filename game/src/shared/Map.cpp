#include <cstring>
#include "Map.h"

Map::Map(Client* client, int size){
    this->size = size;
    this->offset = int((this->size * 4) / 3);
    this->start_x = int(client->getDimension("width") / 2 - (cols / 2) * offset);
    this->start_y = int(client->getDimension("height") / 2 - (rows / 2) * offset);

}
int Map::getSquareState(int x, int y) {
    return this->map[x][y];
}
void Map::setSquareState(int x, int y, int state) {
    this->map[x][y]=state;
}

void Map::drawMap(Client *client) {
    Color c;
    for (int x = 0; x < this->cols; x++) {
        for (int y = 0; y < this->rows; y++) {
            if(this->map[x][y]==0) c = WHITE;
            else if(this->map[x][y]==1) c = BLACK;
            else if(this->map[x][y]==2) c = DARKGRAY;
            DrawRectangle(x * this->offset + this->start_x, y * this->offset + this->start_y, this->size, this->size, c);
        }
    }

}

void Map::debug() {
    for (int x = 0; x < this->cols; x++) {
        for (int y = 0; y < this->rows; y++) {
            const char *info = std::to_string(this->map[x][y]).c_str();
            DrawText(info, x * this->offset + this->start_x, y * this->offset + this->start_y, 10, YELLOW);
        }
    }
}
