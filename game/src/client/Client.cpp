#include "Client.h"
#include <algorithm>
#include <iostream>
#include <thread>

#include "ServerHandler.h"

void Client::Run() const {

  EntityHandler entityHandler;
  ServerHandler serverHandler;

  Map map(25, this->width, this->height);

  std::shared_ptr<int[]> local_boomberman_position(new int[2]);

  serverHandler.connect2Server("127.0.0.1", 2137);
  serverHandler.menu(this->width, this->height);
  serverHandler.wait4Game(entityHandler, this->width, this->height);

  Boomberman *local_boomberman = &entityHandler.players[0];
  Color tutorial_color = local_boomberman->color;
  std::string true_local_name =
      local_boomberman
          ->pseudonim_artystyczny_według_którego_będzie_się_identyfikował_wśród_społeczności_graczy;

  std::unique_ptr<GameMessage> msg;
  std::thread socketReaderThread([&entityHandler, &serverHandler]() {
    serverHandler.receiveLoop(entityHandler);
  });
  socketReaderThread.detach();

  SetTargetFPS(30);
  while (!WindowShouldClose()) {
    local_boomberman_position[0] = local_boomberman->getBoombermanPos()[0];
    local_boomberman_position[1] = local_boomberman->getBoombermanPos()[1];

    // entityHandler.players[0].decrementIframes();

    entityHandler.tryExtinguish();
    if (local_boomberman
            ->pseudonim_artystyczny_według_którego_będzie_się_identyfikował_wśród_społeczności_graczy ==
        true_local_name) {
      if (IsKeyPressed(KEY_SPACE)) {
        entityHandler.bombs.emplace_back(local_boomberman_position[0],
                                         local_boomberman_position[1], 3, 25,
                                         Util::TimestampMillis(), 3, false);
        serverHandler.conn->SendIPlaceBomb(local_boomberman_position[0],
                                           local_boomberman_position[1]);
      }
      if (IsKeyPressed(KEY_RIGHT)) {
        if (local_boomberman->move(&map, local_boomberman_position, 1, 0)) {
          serverHandler.conn->SendIMove(local_boomberman_position[0] + 1,
                                        local_boomberman_position[1]);
        }
      }
      if (IsKeyPressed(KEY_LEFT)) {
        if (local_boomberman->move(&map, local_boomberman_position, -1, 0)) {
          serverHandler.conn->SendIMove(local_boomberman_position[0] - 1,
                                        local_boomberman_position[1]);
        }
      }
      if (IsKeyPressed(KEY_UP)) {
        if (local_boomberman->move(&map, local_boomberman_position, 0, -1)) {
          serverHandler.conn->SendIMove(local_boomberman_position[0],
                                        local_boomberman_position[1] - 1);
        }
      }
      if (IsKeyPressed(KEY_DOWN)) {
        if (local_boomberman->move(&map, local_boomberman_position, 0, 1)) {
          serverHandler.conn->SendIMove(local_boomberman_position[0],
                                        local_boomberman_position[1] + 1);
        }
      }
    }

    BeginDrawing();

    ClearBackground(DARKGRAY);
    Client::drawMap(&map);
    entityHandler.drawFire(&map);
    entityHandler.drawPlayers(&map);
    entityHandler.drawBombs(&map);
    DrawText("Use Arrow Keys to ", 10, 10, 20, LIGHTGRAY);
    DrawText("MOVE", 213, 10, 20, tutorial_color);
    EndDrawing();
  }
  serverHandler.conn->SendILeave();
  local_boomberman->cleanUp();
  delete[] local_boomberman;
}

Client::Client(int width, int height) {
  this->width = width;
  this->height = height;

  InitWindow(width, height, "Boomberman client");
}

int Client::getDimension(const std::string &dimension) const {
  if (dimension == "height")
    return this->height;
  else if (dimension == "width")
    return this->width;
  else
    return -1;
}

Client::~Client() {
  std::cout << "Closing window..." << std::endl;
  CloseWindow();
}

void Client::drawMap(Map *map) {
  Color c;
  for (int x = 0; x < map->cols; x++) {
    for (int y = 0; y < map->rows; y++) {
      int squareState = map->getSquareState(x, y);
      if (squareState == 0)
        c = WHITE;
      else if (squareState == 1)
        c = BLACK;
      else if (squareState == 2)
        c = GRAY;
      DrawRectangle(x * map->offset + map->start_x,
                    y * map->offset + map->start_y, map->size, map->size, c);
    }
  }
}
