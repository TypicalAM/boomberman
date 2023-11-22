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


    std::shared_ptr<int[]> local_boomberman_position(new int[2]);
    while (!WindowShouldClose()) {
        local_boomberman_position[0]=local_boomberman.getBoombermanPos()[0];
        local_boomberman_position[1]=local_boomberman.getBoombermanPos()[1];

        for(auto tile: entityHandler.theFloorIsLava){
            if(local_boomberman_position[0]==tile.x && local_boomberman_position[1]==tile.y) printf("Local player got hit!\n");
        }
        //TODO maybe put this ^^^ into a function somehow, right now there is some include issue

        entityHandler.tryExtinguish();
        if(IsKeyPressed(KEY_SPACE)){
            //entityHandler.destroyWall(&map,0,1);
            entityHandler.placeBomb(local_boomberman_position[0],local_boomberman_position[1],3,25,3.0f);
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
        map.drawMap(this);
        entityHandler.drawWalls(&map);
        entityHandler.drawFire(&map);
        entityHandler.drawPlayers(&map);
        entityHandler.drawBombs(&map);
        map.debug();
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






