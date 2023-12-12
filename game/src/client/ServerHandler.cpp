#include "ServerHandler.h"
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
            if (key == KEY_BACKSPACE && !username.empty()) username.pop_back();
            else if (key == KEY_ENTER && !username.empty()) break;
            else username += (char)key;
        }
        //TODO: BUG WHEN PRESSING BACKSPACE AS A FIRST CHAR

        BeginDrawing();

        ClearBackground(DARKGRAY);
        DrawRectangleRec(textBox, LIGHTGRAY);
        DrawText("Type your username and press ENTER",10,10,20,LIGHTGRAY);
        DrawText(username.c_str(), textBox.x + 5, textBox.y + 10, 20, DARKGRAY);

        EndDrawing();
    }
    return username;
}

void ServerHandler::getRoomList(const char *player_name) { // TODO: ACTUALLY HANDLE THE ROOM LIST
    std::optional<int> bytes_sent = Channel::Send(this->sock, Builder::GetRoomList());
    while (true) {
        this->msg = Channel::Receive(this->sock).value();
        if (this->msg->type() == ROOM_LIST) break;
    }

    auto rl = this->msg->roomlist();
    if (rl.rooms_size() == 0) {
        Channel::Send(this->sock, Builder::JoinRoom(player_name));
        printf("WE ARE THE FIRST ROOM THAT HAS EVER EXISTd BATMAN!\n");
    } else {
        auto room_name = rl.rooms(0).name();
        Channel::Send(this->sock, Builder::JoinRoom(player_name, room_name));
        printf("WE ARE JOINGING ANY ROOM: %s\n", room_name.c_str());
    };
}

void ServerHandler::wait4Game(EntityHandler &eh) {
    while (!WindowShouldClose()) {
        this->msg = Channel::Receive(sock).value();
        printf("%d\n", this->msg->type());
        if (this->msg->type() == GAME_START)
            break;
        else if (this->msg->type() == WELCOME_TO_ROOM)
            this->joinRoom(eh);
        else if (this->msg->type() == GAME_JOIN)
            this->addPlayer(this->msg->gamejoin().player(), eh);

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
