#include <iostream>
#include <cstring>
#include <algorithm>
#include "Client.h"
#include "raylib.h"

void Client::Run() {
    EntityHandler entityHandler;

    Map map(this,25);
    entityHandler.placeWalls(&map);

    Boomerman local_boomberman(0,1,1,3);
    entityHandler.players.push_back(local_boomberman);

    int* current_local_Boomerman_pos;

    while (!WindowShouldClose()) {
        current_local_Boomerman_pos = local_boomberman.getBoomermanPos();
        if(IsKeyPressed(KEY_SPACE)){
           entityHandler.destroyWall(0,0);
        }
        if (IsKeyPressed(KEY_RIGHT)) {
            local_boomberman.move(&map, current_local_Boomerman_pos, 1, 0);
        }
        if (IsKeyPressed(KEY_LEFT)) {
            local_boomberman.move(&map, current_local_Boomerman_pos, -1, 0);
        }
        if (IsKeyPressed(KEY_UP)) {
            local_boomberman.move(&map, current_local_Boomerman_pos, 0, -1);
        }
        if (IsKeyPressed(KEY_DOWN)) {
            local_boomberman.move(&map, current_local_Boomerman_pos, 0, 1);
        }


        BeginDrawing();

        ClearBackground(LIGHTGRAY);
        map.drawMap(this);
        entityHandler.drawWalls(&map);
        entityHandler.drawPlayers(&map);
        map.debug();
        DrawText("Use Arrow Keys to Move", 10, 10, 20, DARKGRAY);

        EndDrawing();
    }
    delete[] current_local_Boomerman_pos;
}

Map::Map(Client* client, int size){
    this->size = size;
    memset( this->map, 0, sizeof(this->map) );

    for (int x = 0; x < this->cols; x++) {
        for (int y = 0; y < this->rows; y++) {
            if(x==0 || x==this->cols-1 || y==0 || y==this->rows-1) this->map[x][y]=1;
        }
    }

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

    for (int x = 0; x < this->cols; x++) {
        for (int y = 0; y < this->rows; y++) {
            DrawRectangle(x * this->offset + this->start_x, y * this->offset + this->start_y, this->size, this->size, GRAY);
            const char *info = std::to_string(this->map[x][y]).c_str();
            DrawText(info, x * this->offset + this->start_x, y * this->offset + this->start_y, 10, YELLOW);
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

int *Map::getColsNrows() {
    int* colsNrows = new int[2];
    colsNrows[0]=this->cols;
    colsNrows[1]=this->rows;
    return colsNrows;
}

Boomerman::Boomerman(int id, int start_x, int start_y, int health) {
    this->position=new int[2];
    this->position[0]=start_x;
    this->position[1]=start_y;
    this->health=health;
    this->id=id;
}
int* Boomerman::getBoomermanPos() const{
    int* pos = new int[2];
    pos[0]=this->position[0];
    pos[1]=this->position[1];
    return pos;
}
void Boomerman::setBoomermanPos(int new_x, int new_y) {
    this->position[0]=new_x;
    this->position[1]=new_y;
}
void Boomerman::move(Map* map, int* curr_pos, int x, int y) {
    int new_x = curr_pos[0]+x;
    int new_y = curr_pos[1]+y;
    int is_wall = map->getSquareState(new_x,new_y);
    if(is_wall!=1) this->setBoomermanPos(new_x, new_y);
}

void Boomerman::shitYourself(Map *map) {
}

Client::Client(int width, int height) {
    this->width = width;
    this->height = height;

    InitWindow(width, height, "Boomberman client");
    SetTargetFPS(60);
}
int Client::getDimension(const std::string& dimension) const {
    if(dimension=="height") return this->height;
    else if(dimension=="width") return this->width;
    else return -1;
}
Client::~Client() {
    std::cout << "Closing window..." << std::endl;
    CloseWindow();
}

Bomb::Bomb(int pos_x, int pos_y, int explosion, int size, float ttl) {
    this->pos_x=pos_x;
    this->pos_y=pos_y;
    this->explosion=explosion;
    this->size=size;
    this->ttl=ttl;
    this->animation_start=GetTime();
    this->plant_time=this->animation_start;
    this->state=-1;
}

void Bomb::animateOrBoom() {
    double now=GetTime();
    if(now-this->animation_start>=0.5f){
        this->state*=-1;
        this->animation_start=now;
    }
    if(now-this->plant_time>=this->ttl) this->should_explode=true;
}

Wall::Wall(int pos_x, int pos_y, bool is_destructible) {
    this->pos_x=pos_x;
    this->pos_y=pos_y;
    this->is_destructible=is_destructible;
}

int* Wall::getPosition() {
    int* positions = new int[2];
    positions[0]=this->pos_x;
    positions[1] = this->pos_y;

    return positions;
}


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

int EntityHandler::destroyWall(int x, int y) {
    auto it = std::find_if(this->walls.begin(), this->walls.end(), [x,y](Wall& wall) {
        return wall.getPosition()[0] == x && wall.getPosition()[1] == y;
    });
    if (it != this->walls.end()) {
        this->walls.erase(it);
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
        positions = player.getBoomermanPos();
        DrawRectangle(positions[0] * map->offset + map->start_x, positions[1] * map->offset + map->start_y, map->size,map->size, BLUE);
    }
    delete[] positions;
}