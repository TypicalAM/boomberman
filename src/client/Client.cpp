#include <iostream>
#include <cstring>
#include "Client.h"
#include "raylib.h"
#include <array>

void Client::Run() {
    Map map(25);
    Boomerman local_boomerman(1,1,3);
    int* current_local_Boomerman_pos;

    while (!WindowShouldClose()) {
        current_local_Boomerman_pos = local_boomerman.getBoomermanPos();
        if (IsKeyPressed(KEY_RIGHT)) {
            local_boomerman.move(&map, current_local_Boomerman_pos, 1, 0);
        }
        if (IsKeyPressed(KEY_LEFT)) {
            local_boomerman.move(&map, current_local_Boomerman_pos, -1, 0);
        }
        if (IsKeyPressed(KEY_UP)) {
            local_boomerman.move(&map, current_local_Boomerman_pos, 0, -1);
        }
        if (IsKeyPressed(KEY_DOWN)) {
            local_boomerman.move(&map, current_local_Boomerman_pos, 0, 1);
        }

        map.localMapUpdate(local_boomerman.getBoomermanPos());

        BeginDrawing();
        ClearBackground(LIGHTGRAY);
        map.drawMap(this);
        DrawText("Use Arrow Keys to Move", 10, 10, 20, DARKGRAY);
        EndDrawing();
    }
}

Map::Map(int size){
    this->size = size;
    memset( this->map, 0, sizeof(this->map) );

    for (int x = 0; x < this->cols; x++) {
        for (int y = 0; y < this->rows; y++) {
            if(x==0 || x==this->cols-1 || y==0 || y==this->rows-1) this->map[x][y]=1;
        }
    }
}
int Map::getSquareState(int x, int y) {
    return this->map[x][y];
}
void Map::setSquareState(int x, int y, int state) {
    this->map[x][y]=state;
}
void Map::localMapUpdate(const int *pos) {
    int x = pos[0];
    int y = pos[1];
    this->map[x][y]=2;
}
void Map::drawMap(Client *client) {
    int offset = int((size*4)/3);
    int start_x = int(client->getDimension("width")/2 - (cols/2)*offset);
    int start_y = int(client->getDimension("height")/2 - (rows/2)*offset);

    auto color = GRAY;
    for (int x = 0; x < this->cols; x++) {
        for (int y = 0; y < this->rows; y++) {
            if(this->map[x][y]==1) color = BLACK;
            else if(this->map[x][y]==2) color = RED;
            else color = GRAY;
            DrawRectangle(x * offset + start_x, y * offset + start_y, this->size, this->size, color);
        }
    }
}

void Map::addBomb(int* pos) {
    Bomb newBomb(pos,3,10,3.0);
    this->bombs.push_back(newBomb);
}

Boomerman::Boomerman(int start_x, int start_y, int health) {
    this->position=new int[2];
    this->position[0]=start_x;
    this->position[1]=start_y;
    this->health=health;
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
    if(map->getSquareState(new_x,new_y)!=1){
        this->setBoomermanPos(new_x,new_y);
        map->setSquareState(curr_pos[0],curr_pos[1],0);
    }
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

Bomb::Bomb(int* pos, int explosion, int size, float ttl) {
    this->position = new int[2];
    this->position=pos;
    this->explosion=explosion;
    this->size=size;
    this->ttl=ttl;
}