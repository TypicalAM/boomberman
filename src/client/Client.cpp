#include <iostream>
#include <cstring>
#include "Client.h"
#include "raylib.h"
#include <array>

void Client::Run() {
    Map map(25);

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(LIGHTGRAY);
        map.drawMap(this);
        EndDrawing();
    }
}

int Boomerman::getBoomermanPosX(){
    return this->player_x;
}
int Boomerman::getBoomermanPosY(){
    return this->player_y;
}

void Map::updateMap(const std::vector<std::array<int, 2>>& player_positions) {
    for(const auto& pos : player_positions){
        int x=pos[0];
        int y=pos[1];

        this->map[x][y]=2;
    }
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

Map::Map(int size){
    this->size = size;
    memset( this->map, 0, sizeof(this->map) );

    for (int x = 0; x < this->cols; x++) {
        for (int y = 0; y < this->rows; y++) {
            if(x==0 || x==this->cols-1 || y==0 || y==this->rows-1) this->map[x][y]=1;
        }
    }
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