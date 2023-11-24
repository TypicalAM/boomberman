#include <iostream>
#include <algorithm>
#include "Client.h"
#include "EntityHandler.h"
#include <iostream>

void Client::Run() {
    EntityHandler entityHandler;

    Map map(this,25);

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
        map.drawMap(this);
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






