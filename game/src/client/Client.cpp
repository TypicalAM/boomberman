#include <iostream>
#include <algorithm>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <iostream>

#include "Client.h"
#include "EntityHandler.h"
#include "raylib.h"
#include "../shared/msg/Channel.h"
#include "../shared/msg/Builder.h"

void Client::Run(const std::string &player_name) const {
    EntityHandler entityHandler;
    Map map(25, this->width, this->height);

    std::shared_ptr<int[]> local_boomberman_position(new int[2]);

    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    sockaddr_in addr{
            .sin_family = PF_INET,
            .sin_port = htons(2137),
            .sin_addr = {inet_addr("127.0.0.1")}};

    int fail = connect(sock, (sockaddr *) &addr, sizeof(addr));

    if (fail) {
        perror("Dupa while connecting");
        exit(1);
    }

    std::optional<int> bytes_sent =
            Channel::Send(sock, Builder::GetRoomList());

    std::unique_ptr<GameMessage> message;
    while (true) {
        message = Channel::Receive(sock).value();
        if (message->type() == ROOM_LIST) break;
    }

    auto rl = message->roomlist();
    if (rl.rooms_size() == 0) {
        Channel::Send(sock, Builder::JoinRoom(player_name));
        printf("WE AHRE THE FIRST ROOM THAT HAS EVER EXISTd BATMAN!\n");
    } else {
        auto room_name = rl.rooms(0).name();
        Channel::Send(sock, Builder::JoinRoom(player_name, room_name));
        printf("WE ARE JOINGING THE FISRT ROOM THAT HAS EVER BEEN SESEN: %s\n", room_name.c_str());
    };

    int start_x, start_y;
    Color start_color;
    while (true) {
        message = Channel::Receive(sock).value();
        printf("%d\n", message->type());
        if (message->type() == GAME_START) break;
        else if (message->type() == WELCOME_TO_ROOM) {
            // We joined the game! YAY!

            // TODO: Look at the welcome_to_room comment in .proto
            auto wtr = message->welcometoroom();
            for (const auto &player: wtr.players()) {
                switch (player.color()) {
                    case PLAYER_RED:
                        start_x = 1;
                        start_y = 1;
                        start_color = RED;
                        break;
                    case PLAYER_GREEN:
                        start_x = 9;
                        start_y = 15;
                        start_color = GREEN;
                        break;
                    case PLAYER_BLUE:
                        start_x = 1;
                        start_y = 15;
                        start_color = BLUE;
                        break;
                    case PLAYER_YELLOW:
                        start_x = 9;
                        start_y = 1;
                        start_color = YELLOW;
                }

                printf("%s\n", player.username().c_str());
                if (!entityHandler.players.empty()) {
                    printf("Player vector not empty!\n");
                    entityHandler.players.emplace_back(player.username().c_str(), start_color, start_x, start_y, 3);
                }
            }
        }
    }
    Boomberman local_boomberman(player_name, start_color, start_x, start_y, 3);
    printf("Player at x:%d y:%d \n", start_x, start_y);
    entityHandler.players.push_back(local_boomberman);

    printf("YOUPIIIIII\n");

    while (!WindowShouldClose()) {
        local_boomberman_position[0] = local_boomberman.getBoombermanPos()[0];
        local_boomberman_position[1] = local_boomberman.getBoombermanPos()[1];

        for (auto tile: entityHandler.theFloorIsLava) {
            if (local_boomberman_position[0] == tile.x && local_boomberman_position[1] == tile.y &&
                entityHandler.players[0].iframes == 0) {
                entityHandler.players[0].gotHit();
            }
        }
        //TODO maybe put this ^^^ into a function somehow, right now there is some include issue

        entityHandler.players[0].decrementIframes();

        entityHandler.tryExtinguish();
        if (IsKeyPressed(KEY_SPACE)) {
            entityHandler.placeBomb(local_boomberman_position[0], local_boomberman_position[1], 3, 25, 3.0f, false);
        }
        if (IsKeyPressed(KEY_RIGHT)) {
            local_boomberman.move(&map, local_boomberman_position, 1, 0);
        }
        if (IsKeyPressed(KEY_LEFT)) {
            local_boomberman.move(&map, local_boomberman_position, -1, 0);
        }
        if (IsKeyPressed(KEY_UP)) {
            local_boomberman.move(&map, local_boomberman_position, 0, -1);
        }
        if (IsKeyPressed(KEY_DOWN)) {
            local_boomberman.move(&map, local_boomberman_position, 0, 1);
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

int Client::getDimension(const std::string &dimension) const {
    if (dimension == "height") return this->height;
    else if (dimension == "width") return this->width;
    else return -1;
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
            if (squareState == 0) c = WHITE;
            else if (squareState == 1) c = BLACK;
            else if (squareState == 2) c = DARKGRAY;
            DrawRectangle(x * map->offset + map->start_x, y * map->offset + map->start_y, map->size, map->size, c);
        }
    }
}






