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

void EntityHandler::placeBomb(int x, int y, int explostion, int size, int ttl) {
    this->bombs.emplace_back(x,y,explostion,size,ttl);
    printf("Placed bomb at x:%d y:%d!\n",x,y);
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
        DrawRectangle(positions[0] * map->offset + map->start_x+3, positions[1] * map->offset + map->start_y+3, map->size-6,map->size-6, BLUE);
    }
    delete[] positions;
}

void EntityHandler::drawBombs(Map* map) {
    Color c;
    for (auto &bomb: this->bombs) {
        if(!bomb.should_explode) {
            bomb.animateOrBoom();
            if (bomb.state == -1) c = BLACK;
            else c = RED;
            DrawRectangle(bomb.pos_x * map->offset + map->start_x + 7, bomb.pos_y * map->offset + map->start_y + 7,
                          map->size - 14, map->size - 14, c);
        }
        else this->explodeBomb(&bomb, map);
    }
}

int EntityHandler::explodeBomb(Bomb* bomb, Map* map) {
    int x=bomb->pos_x, y=bomb->pos_y;
    double fuze_time=bomb->plant_time;
    auto it = std::find_if(this->bombs.begin(), this->bombs.end(), [x,y,fuze_time](Bomb& bomb) {
        return bomb.pos_x == x && bomb.pos_y == y && bomb.plant_time == fuze_time;
    });
    if (it != this->bombs.end()) {
        this->setOnFire(bomb, map);
        this->bombs.erase(it);
        return 0;
    }
    return 1;
}

void EntityHandler::setOnFire(Bomb *bomb, Map* map) {
    this->theFloorIsLava.emplace_back(bomb->pos_x,bomb->pos_y);
    for(int direction=-1; direction<=1; direction+=2) {
        for (int x = 1; x < bomb->explosion; x++) {
            if (map->getSquareState(bomb->pos_x + x*direction, bomb->pos_y)) break;
            this->theFloorIsLava.emplace_back(bomb->pos_x+x*direction, bomb->pos_y);
        }
        for (int y = 1; y < bomb->explosion; y++) {
            if (map->getSquareState(bomb->pos_x, bomb->pos_y + y*direction)) break;
            this->theFloorIsLava.emplace_back(bomb->pos_x, bomb->pos_y+y*direction);
        }
    }
}

void EntityHandler::drawFire(Map *map) {
    for (auto tile: this->theFloorIsLava) {
        DrawRectangle(tile.x * map->offset + map->start_x, tile.y * map->offset + map->start_y, map->size,map->size, RED);
    }
}

void EntityHandler::tryExtinguish() {
    for(size_t i=0; i<this->theFloorIsLava.size(); i++){
        if(this->theFloorIsLava[i].shouldCalmDown()) this->theFloorIsLava.erase(this->theFloorIsLava.begin()+i);
    }
}