#include <iostream>
#include "Client.h"
#include "raylib.h"

void Client::Run() {
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawText("HAHAHAHAHAHAHAHAHAHAHAHAHAAHAH!", 190, 200, 20, LIGHTGRAY);
        EndDrawing();
    }
}

Client::Client(int width, int height) {
    this->width = width;
    this->height = height;
    InitWindow(width, height, "Boomberman client");
    SetTargetFPS(60);
}

Client::~Client() {
    std::cout << "Closing window..." << std::endl;
    CloseWindow();
}