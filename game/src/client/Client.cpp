#include <iostream>
#include <algorithm>
#include "Client.h"
#include "EntityHandler.h"

void Client::Run() {
    EntityHandler entityHandler;

    Map map(this,25);
    entityHandler.placeWalls(&map);

    Boomberman local_boomberman(0,1,1,3);
    entityHandler.players.push_back(local_boomberman);

    int* current_local_Boomberman_pos;

    while (!WindowShouldClose()) {
        current_local_Boomberman_pos = local_boomberman.getBoombermanPos();
        if(IsKeyPressed(KEY_SPACE)){
           entityHandler.destroyWall(&map,0,1);
        }
        if (IsKeyPressed(KEY_RIGHT)) {
            local_boomberman.move(&map, current_local_Boomberman_pos, 1, 0);
        }
        if (IsKeyPressed(KEY_LEFT)) {
            local_boomberman.move(&map, current_local_Boomberman_pos, -1, 0);
        }
        if (IsKeyPressed(KEY_UP)) {
            local_boomberman.move(&map, current_local_Boomberman_pos, 0, -1);
        }
        if (IsKeyPressed(KEY_DOWN)) {
            local_boomberman.move(&map, current_local_Boomberman_pos, 0, 1);
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
    delete[] current_local_Boomberman_pos;
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






