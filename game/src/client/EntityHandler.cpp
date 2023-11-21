#include <algorithm>
#include "EntityHandler.h"

void EntityHandler::placeWalls(Map *map) {
    int* colsNrows = map->getColsNrows();
    for(int x=0; x<colsNrows[0]; x++){
        for(int y=0; y<colsNrows[1]; y++){
            if(map->getSquareState(x,y)==1){
                this->walls.emplace_back(x,y,false);
            }
        }
    }
    delete[] colsNrows;
}

int EntityHandler::destroyWall(Map* map, int x, int y) {
    auto it = std::find_if(this->walls.begin(), this->walls.end(), [x,y](Wall& wall) {
        return wall.getPosition()[0] == x && wall.getPosition()[1] == y;
    });
    if (it != this->walls.end()) {
        this->walls.erase(it);
        map->setSquareState(x,y,0);
        return 0;
    }
    return 1;
}

void EntityHandler::drawWalls(Map* map) {
    int* positions;
    for (auto wall: this->walls) {
        positions = wall.getPosition();
        DrawRectangle(positions[0] * map->offset + map->start_x, positions[1] * map->offset + map->start_y, map->size,map->size, BLACK);
    }
    delete[] positions;
}

void EntityHandler::drawPlayers(Map *map) {
    int* positions;
    for (auto player: this->players) {
        positions = player.getBoombermanPos();
        DrawRectangle(positions[0] * map->offset + map->start_x, positions[1] * map->offset + map->start_y, map->size,map->size, BLUE);
    }
    delete[] positions;
}
