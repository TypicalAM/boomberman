#include "Client.h"
#include <cmath>
#include <iostream>
#include <thread>
#include <unistd.h>

#include "ServerHandler.h"
#include "raylib.h"

std::atomic<bool> already_waiting = false;

void waitThreeSeconds(ServerHandler *sh) {
  std::this_thread::sleep_for(std::chrono::seconds(3));
  shutdown(sh->conn->sock, SHUT_RDWR);
  close(sh->conn->sock);
}

void Client::Run(const std::string &server, int port) const {
  EntityHandler entityHandler;
  ServerHandler serverHandler;

  Map map(25, this->width, this->height);

  std::shared_ptr<float[]> local_boomberman_position(new float[2]);

  serverHandler.connect2Server(server.c_str(), port);
  serverHandler.menu(this->width, this->height);
  std::cout << "Going at menu" << std::endl;
  serverHandler.wait4Game(entityHandler);
  std::cout << "Going at 4 for game" << std::endl;

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
    local_boomberman_position[0] =
        std::floor(local_boomberman->getBoombermanPos()[0]);
    local_boomberman_position[1] =
        std::floor(local_boomberman->getBoombermanPos()[1]);

    entityHandler.tryExtinguish();
    if (local_boomberman
            ->pseudonim_artystyczny_według_którego_będzie_się_identyfikował_wśród_społeczności_graczy ==
        true_local_name) {
      if (IsKeyPressed(KEY_SPACE)) {
        entityHandler.bombs.emplace_back(local_boomberman_position[0],
                                         local_boomberman_position[1], 3, 25,
                                         util::TimestampMillis(), 3, false);
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
      if (IsKeyPressed(KEY_ESCAPE)) {
        serverHandler.conn->SendILeave();
        shutdown(serverHandler.conn->sock, SHUT_RDWR);
        close(serverHandler.conn->sock);
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
    if (!serverHandler.winner.empty()) {
      std::string winText = "Player " + serverHandler.winner + " won!";
      DrawText(winText.c_str(), 10, int(height) - 30, 30, GREEN);
      if (!already_waiting) {
        std::thread(&waitThreeSeconds, &serverHandler).detach();
        already_waiting.store(true);
      }
    }
    EndDrawing();
  }
  local_boomberman->cleanUp();
}

Client::Client(float width, float height) {
  this->width = width;
  this->height = height;

  InitWindow(std::floor(width), std::floor(height), "Boomberman client");
}

Client::~Client() {}

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
