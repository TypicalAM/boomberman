#include "ServerHandler.h"
#include "RoomBox.h"
#include <algorithm>
#include <memory>
#include <unistd.h>

ServerHandler::ServerHandler() {
  this->conn =
      std::make_unique<Connection>(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));

  this->polling[0].fd = this->conn->sock;
  this->polling[0].events = POLLIN | POLLOUT;
}

void ServerHandler::connect2Server(const char *ip, int port) const {
  sockaddr_in addr{};
  addr.sin_port = htons(port);
  addr.sin_family = PF_INET;
  addr.sin_addr.s_addr = inet_addr(ip);

  if (connect(this->conn->sock, (sockaddr *)&addr, sizeof(addr))) {
    if (errno != EINPROGRESS) {
      perror("Connection to the server failed!");
      exit(1);
    }
  }
}

void ServerHandler::handleMessage(EntityHandler &eh,
                                  std::unique_ptr<GameMessage> msg) {
  std::cout << "Got msg of type: " << msg->type() << std::endl;
  switch (msg->type()) {
  case OTHER_MOVE: {
    std::string username = msg->othermove().username();
    if (username.empty())
      return;
    std::cout << "Client handling OTHER_MOVE for " << username << std::endl;
    auto found = ServerHandler::findPlayer(eh, username);
    if (found != eh.players.end()) {
      found->setBoombermanPos(int(msg->othermove().x()),
                              int(msg->othermove().y()));
    }
    break;
  }
  case OTHER_LEAVE: {
    std::cout << "Client handling OTHER_LEAVE" << std::endl;
    std::string username = msg->otherleave().username();
    std::cout << "Client: " << username << std::endl;
    auto found = ServerHandler::findPlayer(eh, username);
    if (found != eh.players.end())
      eh.players.erase(found);
    break;
  }
  case GOT_HIT: {
    std::string username = msg->gothit().username();
    std::cout << "Client handling GOT_HIT for player " << username << std::endl;
    auto found = ServerHandler::findPlayer(eh, username);
    if (found != eh.players.end()) {
      found->gotHit(msg->gothit().timestamp());
      if (msg->gothit().livesremaining() <= 0)
        eh.players.erase(found);
    }
    break;
  }
  case OTHER_BOMB_PLACE: {
    std::cout << "Client handling OTHER_BOMB_PLACE" << std::endl;
    if (msg->otherbombplace().username() == "Server") {
      eh.bombs.emplace_back(msg->otherbombplace().x(),
                            msg->otherbombplace().y(), 9, 25,
                            msg->otherbombplace().timestamp(), 3, true);
    } else {
      eh.bombs.emplace_back(msg->otherbombplace().x(),
                            msg->otherbombplace().y(), 3, 25,
                            msg->otherbombplace().timestamp(), 3, false);
    }
    break;
  }

  case MOVEMENT_CORRECTION: {
    std::cout << "Got movement correction orders. I could have been much "
                 "speedy... Sir yes Sir!"
              << std::endl;
    eh.players[0].setBoombermanPos(int(msg->movementcorrection().x()),
                                   int(msg->movementcorrection().y()));
    break;
  }

  case GAME_WON: {
    winner = msg->gamewon().winnerusername();
    std::cout << "Game won by " << winner << std::endl;
  }
  default:
    break;
  }
}

void ServerHandler::receiveLoop(EntityHandler &eh) {
  printf("Receive loop running\n");
  while (true) {
    int ready = poll(this->polling, 1, -1);
    if (ready == -1) {
      shutdown(this->conn->sock, SHUT_RDWR);
      close(this->conn->sock);
      error(1, errno, "poll failed");
    }

    if (polling[0].revents & POLLIN) {
      if (!this->conn)
        return;
      auto msg = this->conn->Receive();
      if (!msg.has_value()) {
        std::cout << "The server has thanked us, grace to him and the party"
                  << std::endl;
        exit(0);
      }

      this->handleMessage(eh, std::move(msg.value()));

      while (this->conn->HasMoreMessages()) {
        auto msg = this->conn->Receive();
        if (!msg.has_value()) {
          std::cout << "The server has thanked us, grace to him and the party"
                    << std::endl;
          exit(0);
        }

        this->handleMessage(eh, std::move(msg.value()));
      }
    }
  }
}

std::string ServerHandler::selectUsername(float screen_width,
                                          float screen_height) {
  std::string username;
  Rectangle textBox = {screen_width / 2 - 100, screen_height / 2 - 100, 200,
                       40};
  int invalid_chars[] = {32,  256, 257, 258, 259, 260, 261, 262, 263, 264,
                         265, 266, 267, 268, 269, 280, 281, 282, 283, 284,
                         340, 341, 342, 343, 344, 345, 346, 347, 348};

  bool invalid_found;
  while (!WindowShouldClose()) {
    int key = GetKeyPressed();

    if (key != 0) {
      if (key == KEY_BACKSPACE) {
        if (strlen(username.c_str()) > 0)
          username.pop_back();
      } else if (key == KEY_ENTER) {
        if (strlen(username.c_str()) > 0)
          break;
      } else {
        invalid_found = false;
        for (int invalid_char : invalid_chars) {
          if (key == invalid_char) {
            invalid_found = true;
            break;
          }
        }
        if (!invalid_found)
          username += (char)key;
      }
    }

    BeginDrawing();

    ClearBackground(DARKGRAY);
    DrawRectangleRec(textBox, LIGHTGRAY);
    DrawText("Type your username and press ENTER", 10, 10, 20, LIGHTGRAY);
    DrawText(username.c_str(), textBox.x + 5, textBox.y + 10, 20, DARKGRAY);

    EndDrawing();
  }
  return username;
}

void ServerHandler::menu(float width, float height) {
  this->conn->SendGetRoomList();

  std::unique_ptr<GameMessage> msg;
  while (true) {
    auto opt_msg = this->conn->Receive();
    if (!opt_msg.has_value()) {
      std::cout << "The server has thanked us, grace to him and the party"
                << std::endl;
      exit(0);
    }

    msg = std::move(opt_msg.value());
    if (msg->type() == ROOM_LIST)
      break;
  }

  auto rl = msg->roomlist();
  int rooms_total = rl.rooms_size();

  Rectangle newGameButton = {(width / 2) - 95, 110, 190, 60};
  auto joinGameButtonColor = GRAY;
  Rectangle joinGameButton = {(width / 2) - 60, 190, 120, 60};

  while (true) {
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
      Vector2 mousePoint = GetMousePosition();
      if (CheckCollisionPointRec(mousePoint, newGameButton)) {
        std::string username = ServerHandler::selectUsername(width, height);
        if (!username.empty()) {
          std::cout << "Sending join room with username: " << username;
          this->conn->SendJoinRoom(username);
        } else {
          std::cout << "USERNAME NOT PROVIDED" << std::endl;
        }

        return;
      }
    }
    BeginDrawing();
    ClearBackground(DARKGRAY);
    DrawText("Welcome to Boomberman!", (width / 2) - 180, 30, 30, LIGHTGRAY);

    DrawRectangleRec(newGameButton, LIGHTGRAY);
    DrawText("Create new game", newGameButton.x + 10, newGameButton.y + 20, 20,
             DARKGRAY);

    if (rooms_total > 0) {
      joinGameButtonColor = LIGHTGRAY;
      if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Vector2 mousePoint = GetMousePosition();
        if (CheckCollisionPointRec(mousePoint, joinGameButton)) {
          EndDrawing();
          return listRooms(width, height, std::move(msg));
        }
      }
    }
    DrawRectangleRec(joinGameButton, joinGameButtonColor);
    DrawText("Join game", joinGameButton.x + 15, joinGameButton.y + 20, 20,
             DARKGRAY);
    EndDrawing();
  }
}

void ServerHandler::listRooms(float width, float height,
                              std::unique_ptr<GameMessage> msg) {
  std::vector<RoomBox> rooms;
  auto rl = msg->roomlist();

  int current_row = -1;
  float offset_from_top = 70;
  for (int i = 0; i < rl.rooms_size(); i++) {
    int column = i % 3;
    if (i % 3 == 0)
      current_row++;
    rooms.emplace_back(110 + column * (width / 4) + 28,
                       current_row * 120 + offset_from_top, 150, 60,
                       rl.rooms(i).name(), rl.rooms(i).playercount());
  }

  Camera2D camera;
  camera.target = {width / 2, height / 2};
  camera.offset = {width / 2, height / 2};
  camera.rotation = 0.0f;
  camera.zoom = 1.0f;

  Rectangle backButton = {10, 10, 80, 30};

  while (true) {
    // Update
    if (IsKeyPressed(KEY_UP) &&
        camera.offset.y <= (height / 2) - offset_from_top)
      camera.offset.y += offset_from_top;
    if (IsKeyPressed(KEY_DOWN))
      camera.offset.y -= offset_from_top;

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
      Vector2 mousePoint = GetMousePosition();

      for (const auto &room : rooms) {
        if (CheckCollisionPointRec(mousePoint, room.rect)) {
          std::string username = ServerHandler::selectUsername(width, height);
          if (!username.empty()) {
            this->conn->SendJoinRoom(username, room.label);
          } else {
            std::cout << "USERNAME NOT PROVIDED" << std::endl;
          }
          return;
        } else if (CheckCollisionPointRec(mousePoint, backButton))
          return this->menu(width, height);
      }
    }
    BeginDrawing();
    BeginMode2D(camera);
    ClearBackground(DARKGRAY);

    DrawText("Select room", (width / 2) - 80, 10, 30, LIGHTGRAY);

    DrawRectangleRec(backButton, LIGHTGRAY);
    DrawText("Back", backButton.x + 5, backButton.y + 2, 30, DARKGRAY);

    for (const auto &room : rooms) {
      DrawRectangleRec(room.rect, LIGHTGRAY);
      DrawText(room.label.c_str(), room.rect.x + 10, room.rect.y + 20, 20,
               DARKGRAY);
      DrawText((std::to_string(room.players_in) + "/4").c_str(),
               room.rect.x + 60, room.rect.y + offset_from_top + 10, 20,
               LIGHTGRAY);
    }
    EndMode2D();
    EndDrawing();
  }
}

void ServerHandler::wait4Game(EntityHandler &eh) {
  while (true) {
    auto opt_msg = this->conn->Receive();
    if (!opt_msg.has_value()) {
      std::cout << "The server has thanked us, grace to him and the party"
                << std::endl;
      exit(0);
    }

    bool esc = handleLobbyMsg(eh, std::move(opt_msg.value()));
    if (esc)
      break;
  }
}

bool ServerHandler::handleLobbyMsg(EntityHandler &eh,
                                   std::unique_ptr<GameMessage> msg) {
  std::cout << " HANDLIGN A MESSAGE" << std::endl;
  if (msg->type() == GAME_START) {
    printf("Starting game with %zu players\n", eh.players.size());
    std::cout << "PLayers in room: ";
    for (auto player : eh.players) {
      std::cout
          << player
                 .pseudonim_artystyczny_według_którego_będzie_się_identyfikował_wśród_społeczności_graczy
          << " ";
    }
    std::cout << std::endl;
    return true;
  } else if (msg->type() == WELCOME_TO_ROOM) {
    this->joinRoom(eh, std::move(msg));
  } else if (msg->type() == GAME_JOIN) {
    printf("GOT GAME JOIN\n");
    std::cout << "Player name: " << msg->gamejoin().player().username()
              << std::endl;
    this->addPlayer(msg->gamejoin().player(), eh);
  } else if (msg->type() == ERROR) {
    std::cout << "GOT EREROR: " << msg->error().error() << std::endl;
    Rectangle backButton = {10, 10, 80, 30};
    while (true) {
      if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Vector2 mousePoint = GetMousePosition();
        if (CheckCollisionPointRec(mousePoint, backButton))
          exit(0);
      }

      BeginDrawing();
      ClearBackground(DARKGRAY);
      DrawRectangleRec(backButton, LIGHTGRAY);
      DrawText("Exit", backButton.x + 5, backButton.y + 2, 30, DARKGRAY);
      DrawText("This username is already taken in this room!", 10, 50, 20, RED);
      EndDrawing();
    }
  }

  BeginDrawing();
  ClearBackground(DARKGRAY);
  DrawText("Waiting for game to start...", 10, 10, 20, LIGHTGRAY);
  EndDrawing();
  return false;
}

void ServerHandler::joinRoom(EntityHandler &eh,
                             std::unique_ptr<GameMessage> msg) {
  auto wtr = msg->welcometoroom();
  for (const auto &player : wtr.players()) {
    this->addPlayer(player, eh);
  }
}

void ServerHandler::addPlayer(const GamePlayer &player, EntityHandler &eh) {
  this->setPlayerParams(player);
  eh.players.emplace_back(std::string(player.username()), this->start_color,
                          this->start_x, this->start_y, 3);
  std::cout << "Added player to local vector: " << player.username()
            << " with start pos (x,y): (" << this->start_x << ","
            << this->start_y << ")" << std::endl;
}

std::vector<Boomberman>::iterator
ServerHandler::findPlayer(EntityHandler &eh, const std::string &username) {
  return std::find_if(
      eh.players.begin(), eh.players.end(), [username](Boomberman &player) {
        return player
                   .pseudonim_artystyczny_według_którego_będzie_się_identyfikował_wśród_społeczności_graczy ==
               username;
      });
}

void ServerHandler::setPlayerParams(const GamePlayer &player) {
  switch (player.color()) {
  case PLAYER_RED:
    this->start_x = 1;
    this->start_y = 1;
    this->start_color = RED;
    break;
  case PLAYER_GREEN:
    this->start_x = 15;
    this->start_y = 1;
    this->start_color = GREEN;
    break;
  case PLAYER_BLUE:
    this->start_x = 1;
    this->start_y = 9;
    this->start_color = BLUE;
    break;
  case PLAYER_YELLOW:
    this->start_x = 15;
    this->start_y = 9;
    this->start_color = YELLOW;
    break;
  default:
    break;
  }
}
