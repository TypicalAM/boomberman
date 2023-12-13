#include "ServerHandler.h"
#include "RoomBox.h"
#include <csignal>

ServerHandler::ServerHandler() {
    this->sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    this->polling[0].fd = this->sock;
    this->polling[0].events = POLLIN | POLLOUT;
}

void ServerHandler::connect2Server(const char *ip, int port) const {
    sockaddr_in addr{.sin_family = PF_INET,
            .sin_port = htons(port),
            .sin_addr = {inet_addr(ip)}};

    if (connect(this->sock, (sockaddr *) &addr, sizeof(addr))) {
        if (errno != EINPROGRESS) {
            perror("Connection to the server failed!");
            exit(1);
        }
    }
}

void ServerHandler::receiveLoop(EntityHandler &eh) {
    printf("Receive loop running\n");
    // if (fcntl(this->sock, F_SETFL, O_NONBLOCK)) perror("fcntl");
    while (1) {
        int ready = poll(this->polling, 1, -1);
        if (ready == -1) {
            shutdown(this->sock, SHUT_RDWR);
            close(this->sock);
            error(1, errno, "poll failed");
        }

        if (polling[0].revents & POLLIN) {
            this->msg = Channel::Receive(this->sock).value();
            printf("%d\n", msg->type());
            switch (this->msg->type()) {
                case OTHER_MOVE: {
                    std::string username = this->msg->othermove().username();
                    auto found = ServerHandler::findPlayer(eh,username);
                    if (found != eh.players.end()) {
                        found->setBoombermanPos(int(this->msg->othermove().x()),
                                             int(this->msg->othermove().y()));
                    }
                    printf("Player %s moved\n",found->pseudonim_artystyczny_według_którego_będzie_się_identyfikował_wśród_społeczności_graczy.c_str());

                    break;
                }
                case OTHER_LEAVE: {
                    std::string username = msg->otherleave().username();
                    auto found = ServerHandler::findPlayer(eh,username);
                    if (found != eh.players.end()) eh.players.erase(found);
                    break;
                }
                case GOT_HIT: {
                    std::string username = msg->gothit().username();
                    auto found = ServerHandler::findPlayer(eh,username);
                    if (found != eh.players.end()){
                        found->gotHit(msg->gothit().timestamp()); //TODO: Adamie, server side iframe'y poproszę
                        if(msg->gothit().livesremaining()<=0) eh.players.erase(found);
                    }
                    break;
                }
                case OTHER_BOMB_PLACE: {
                    if (msg->otherbombplace().username() == "Server") {
                        eh.bombs.emplace_back(
                                this->msg->otherbombplace().x(), this->msg->otherbombplace().y(), 7,
                                25, this->msg->otherbombplace().timestamp(), 3, true);
                    } else {
                        eh.bombs.emplace_back(
                                this->msg->otherbombplace().x(), this->msg->otherbombplace().y(), 3,
                                25, this->msg->otherbombplace().timestamp(), 3, false);
                    }
                    break;
                }
            }
        }
    }
}

std::string ServerHandler::selectUsername(float screen_width, float screen_height) {
    std::string username;
    Rectangle textBox = {screen_width/2 -100, screen_height/2 - 100, 200, 40};
    while(!WindowShouldClose()) {
        int key = GetKeyPressed();

        if(key != 0) {
            if (key == KEY_BACKSPACE) {
                if(strlen(username.c_str())>0) username.pop_back();
            }
            else if (key == KEY_ENTER){
                if(strlen(username.c_str())>0) break;
            }
            else username += (char)key;
        }

        BeginDrawing();

        ClearBackground(DARKGRAY);
        DrawRectangleRec(textBox, LIGHTGRAY);
        DrawText("Type your username and press ENTER",10,10,20,LIGHTGRAY);
        DrawText(username.c_str(), textBox.x + 5, textBox.y + 10, 20, DARKGRAY);

        EndDrawing();
    }
    return username;
}


void ServerHandler::menu(float width, float height) { // TODO: ACTUALLY HANDLE THE ROOM LIST
    std::optional<int> bytes_sent = Channel::Send(this->sock, Builder::GetRoomList());
    while (true) {
        this->msg = Channel::Receive(this->sock).value();
        if (this->msg->type() == ROOM_LIST) break;
    }

    auto rl = this->msg->roomlist();
    int rooms_total = rl.rooms_size();

    Rectangle newGameButton = {(width / 2)-95, 110, 190, 60};
    auto joinGameButtonColor=GRAY;
    Rectangle joinGameButton= {(width / 2)-60, 190, 120, 60};

    while(!WindowShouldClose()) {
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){
            Vector2 mousePoint = GetMousePosition();
            if(CheckCollisionPointRec(mousePoint,newGameButton)){
                Channel::Send(this->sock, Builder::JoinRoom(ServerHandler::selectUsername(width,height)));
                return;
            }
        }
        BeginDrawing();
        ClearBackground(DARKGRAY);
        DrawText("Welcome to Boomberman!", (width / 2) - 180, 30 ,30, LIGHTGRAY);

        DrawRectangleRec(newGameButton,LIGHTGRAY);
        DrawText("Create new game",newGameButton.x+10,newGameButton.y+20,20,DARKGRAY);

        if(rooms_total>0){
            joinGameButtonColor = LIGHTGRAY;
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){
                Vector2 mousePoint = GetMousePosition();
                if(CheckCollisionPointRec(mousePoint,joinGameButton)){
                    EndDrawing();
                    return listRooms(width,height);
                }
            }
        }
        DrawRectangleRec(joinGameButton,joinGameButtonColor);
        DrawText("Join game",joinGameButton.x+15,joinGameButton.y+20,20,DARKGRAY);
        EndDrawing();
    }


}

void ServerHandler::listRooms(float width, float height){
    std::vector<RoomBox> rooms;
    auto rl = this->msg->roomlist();

    int current_row = -1;
    float offset_from_top = 70;
    for (int i = 0; i < rl.rooms_size(); i++) {
        int column = i % 3;
        if (i % 3 == 0) current_row++;
        rooms.emplace_back(110 + column * (width/4) + 28, current_row * 120 + offset_from_top, 150, 60,
                           rl.rooms(i).name(), rl.rooms(i).playercount());
    }

    Camera2D camera = {0};
    camera.target = {width / 2, height / 2};
    camera.offset = {width / 2, height / 2};
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    Rectangle backButton = {10,10,80,30};

    while (!WindowShouldClose()) {
        // Update
        if (IsKeyPressed(KEY_UP) && camera.offset.y <= (height/2)-offset_from_top) camera.offset.y += offset_from_top;
        if (IsKeyPressed(KEY_DOWN)) camera.offset.y -= offset_from_top;

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 mousePoint = GetMousePosition();

            for (const auto& room : rooms) {
                if (CheckCollisionPointRec(mousePoint, room.rect)) {
                    Channel::Send(this->sock, Builder::JoinRoom(ServerHandler::selectUsername(width,height), room.label));
                    return;
                }
                else if(CheckCollisionPointRec(mousePoint, backButton)) return this->menu(width,height);
            }
        }
        BeginDrawing();
        BeginMode2D(camera);
        ClearBackground(DARKGRAY);

        DrawText("Select room", (width / 2) - 80, 10, 30, LIGHTGRAY);

        DrawRectangleRec(backButton, LIGHTGRAY);
        DrawText("Back", backButton.x+5, backButton.y+2, 30, DARKGRAY);


        // Draw rectangles and text inside
        for (const auto &room: rooms) {
            DrawRectangleRec(room.rect, LIGHTGRAY);
            DrawText(room.label.c_str(), room.rect.x + 10, room.rect.y+20, 20, DARKGRAY);
            DrawText((std::to_string(room.players_in) + "/4").c_str(), room.rect.x + 60, room.rect.y + offset_from_top+10, 20, LIGHTGRAY);
        }
        EndMode2D();
        EndDrawing();
    }
}

void ServerHandler::wait4Game(EntityHandler &eh) {
    printf("INSIDE W8TING4GAME!\n");
    while (true) {
        this->msg = Channel::Receive(sock).value();
        if (this->msg->type() == GAME_START) {
            printf("Starting game with %zu players\n", eh.players.size());
            break;
        }
        else if (this->msg->type() == WELCOME_TO_ROOM)
            this->joinRoom(eh);
        else if (this->msg->type() == GAME_JOIN) {
            printf("GOT GAME JOIN\n");
            this->addPlayer(this->msg->gamejoin().player(), eh);
        }

        BeginDrawing();
        ClearBackground(DARKGRAY);
        DrawText("Waiting for game to start...", 10, 10, 20, LIGHTGRAY);
        EndDrawing();
    }
}

void ServerHandler::joinRoom(EntityHandler &eh) {
    auto wtr = this->msg->welcometoroom();
    for (const auto &player: wtr.players()) {
        this->addPlayer(player, eh);
    }
}

void ServerHandler::addPlayer(const GamePlayer &player, EntityHandler &eh) {
    this->setPlayerParams(player);
    eh.players.emplace_back(player.username(), this->start_color, this->start_x,
                            this->start_y, 3);
    std::cout << "Added player to local vector: " << player.username()
              << " with start pos (x,y): (" << this->start_x << ","
              << this->start_y << ")" << std::endl;
}

std::vector<Boomberman>::iterator ServerHandler::findPlayer(EntityHandler &eh, const std::string& username) {
    return std::find_if(
            eh.players.begin(), eh.players.end(),
            [username](Boomberman &player) {
                return player.pseudonim_artystyczny_według_którego_będzie_się_identyfikował_wśród_społeczności_graczy == username;
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
    }
}
