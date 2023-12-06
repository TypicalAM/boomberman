#include "ServerHandler.h"

void ServerHandler::connect2Server(const char *ip, int port) {
    this->sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    sockaddr_in addr{
            .sin_family = PF_INET,
            .sin_port = htons(port),
            .sin_addr = {inet_addr(ip)}};

    int fail = connect(this->sock, (sockaddr *) &addr, sizeof(addr));

    if (fail) {
        perror("Connection to the server failed!");
        exit(1);
    }
}

void ServerHandler::getRoomList(const char* player_name) { //TODO: ACTUALLY HANDLE THE ROOM LIST
    std::optional<int> bytes_sent = Channel::Send(sock, Builder::GetRoomList());
    while (true) {
        this->msg = Channel::Receive(sock).value();
        if (this->msg->type() == ROOM_LIST) break;
    }

    auto rl = this->msg->roomlist();
    if (rl.rooms_size() == 0) {
        Channel::Send(sock, Builder::JoinRoom(player_name));
        printf("WE ARE THE FIRST ROOM THAT HAS EVER EXISTd BATMAN!\n");
    } else {
        auto room_name = rl.rooms(0).name();
        Channel::Send(sock, Builder::JoinRoom(player_name, room_name));
        printf("WE ARE JOINGING ANY ROOM: %s\n", room_name.c_str());
    };
}

void ServerHandler::wait4Game(EntityHandler &eh) {
    while (true) {
        this->msg = Channel::Receive(sock).value();
        printf("%d\n", this->msg->type());
        if (this->msg->type() == GAME_START) break;
        else if (this->msg->type() == WELCOME_TO_ROOM) this->joinRoom(eh);
        else if (this->msg->type() == GAME_JOIN) this->addPlayer(this->msg->gamejoin().player(), eh);

    }
}

void ServerHandler::joinRoom(EntityHandler &eh) {
    auto wtr = this->msg->welcometoroom();
    for (const auto &player: wtr.players()) {
        this->addPlayer(player, eh);
    }
}

void ServerHandler::addPlayer(GamePlayer player, EntityHandler &eh) {
    this->setPlayerParams(player);
    eh.players.emplace_back(player.username(), this->start_color, this->start_x, this->start_y, 3);
    std::cout<<"Added player to local vector: "<<player.username()<<" with start pos (x,y): ("<<this->start_x<<","<<this->start_y<<")"<<std::endl;
}

void ServerHandler::setPlayerParams(const GamePlayer& player) {
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