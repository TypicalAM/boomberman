#include <thread>
#include <iostream>
#include "raylib.h"

const int screenWidth = 800;
const int screenHeight = 450;

int main() {
    std::cout << "Starting program" << std::endl;
    InitWindow(screenWidth, screenHeight, "my basic window");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawText("HAHAHAHAHAHAHAHAHAHAHAHAHAAHAH!", 190, 200, 20, LIGHTGRAY);
        EndDrawing();
    }

    CloseWindow();
    std::cout << "Ended program" << std::endl;
    return 0;
}