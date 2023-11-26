#include <iostream>
#include <algorithm>
#include "Client.h"
#include "EntityHandler.h"
#include "raylib.h"

void Client::Run() const {
    EntityHandler entityHandler;

    Map map(25,this->width,this->height);

    Boomberman local_boomberman("ارهابي",1,1,3);
    entityHandler.players.push_back(local_boomberman);

    std::shared_ptr<int[]> local_boomberman_position(new int[2]);
    while (!WindowShouldClose()) {
        local_boomberman_position[0]=local_boomberman.getBoombermanPos()[0];
        local_boomberman_position[1]=local_boomberman.getBoombermanPos()[1];

        for(auto tile: entityHandler.theFloorIsLava){
            if(local_boomberman_position[0]==tile.x && local_boomberman_position[1]==tile.y && entityHandler.players[0].iframes==0){
                entityHandler.players[0].gotHit();
            }
        }
        //TODO maybe put this ^^^ into a function somehow, right now there is some include issue

        entityHandler.players[0].decrementIframes();

        entityHandler.tryExtinguish();
        if(IsKeyPressed(KEY_SPACE)){
            entityHandler.placeBomb(local_boomberman_position[0],local_boomberman_position[1],3,25,3.0f,false);
        }
        if (IsKeyPressed(KEY_RIGHT)) {
            local_boomberman.move(&map, local_boomberman_position,1, 0);
        }
        if (IsKeyPressed(KEY_LEFT)) {
            local_boomberman.move(&map, local_boomberman_position, -1, 0);
        }
        if (IsKeyPressed(KEY_UP)) {
            local_boomberman.move(&map,local_boomberman_position, 0, -1);
        }
        if (IsKeyPressed(KEY_DOWN)) {
            local_boomberman.move(&map,local_boomberman_position, 0, 1);
        }

        BeginDrawing();

        ClearBackground(LIGHTGRAY);
        Client::drawMap(&map);
        entityHandler.drawFire(&map);
        entityHandler.drawPlayers(&map);
        entityHandler.drawBombs(&map);
        DrawText("Use Arrow Keys to Move", 10, 10, 20, DARKGRAY);

        EndDrawing();
    }
    local_boomberman.cleanUp();
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

void Client::drawMap(Map* map) {
    Color c;
    for (int x = 0; x < map->cols; x++) {
        for (int y = 0; y < map->rows; y++) {
            int squareState = map->getSquareState(x,y);
            if(squareState==0) c = WHITE;
            else if(squareState==1) c = BLACK;
            else if(squareState==2) c = DARKGRAY;
            DrawRectangle(x * map->offset + map->start_x, y * map->offset + map->start_y, map->size, map->size, c);
        }
    }
}






