#include "Builder.h"

std::unique_ptr<GameMessage> Builder::Error(const std::string &error) {
    auto *e = new class Error;
    e->set_error(error);
    GameMessage msg;
    msg.set_type(ERROR);
    msg.set_allocated_error(e);
    return std::make_unique<GameMessage>(msg);
}

std::unique_ptr<GameMessage> Builder::GetRoomList() {
    auto *grl = new class GetRoomList;
    GameMessage msg;
    msg.set_type(GET_ROOM_LIST);
    msg.set_allocated_getroomlist(grl);
    return std::make_unique<GameMessage>(msg);
}

std::unique_ptr<GameMessage> Builder::JoinRoom(const std::string &name, std::optional<std::string> roomName) {
    auto *jr = new class JoinRoom;
    jr->set_username(name);
    if (roomName.has_value()) jr->set_roomname(roomName.value());

    GameMessage msg;
    msg.set_type(JOIN_ROOM);
    msg.set_allocated_joinroom(jr);
    return std::make_unique<GameMessage>(msg);
}

std::unique_ptr<GameMessage> Builder::IPlaceBomb(float x, float y) {
    auto *ipb = new class IPlaceBomb;
    ipb->set_x(x);
    ipb->set_y(y);

    GameMessage msg;
    msg.set_type(I_PLACE_BOMB);
    msg.set_allocated_iplacebomb(ipb);
    return std::make_unique<GameMessage>(msg);
}

std::unique_ptr<GameMessage> Builder::IMove(float x, float y) {
    auto *im = new class IMove;
    im->set_x(x);
    im->set_y(y);

    GameMessage msg;
    msg.set_type(I_MOVE);
    msg.set_allocated_imove(im);
    return std::make_unique<GameMessage>(msg);
}

std::unique_ptr<GameMessage> Builder::ILeave() {
    GameMessage msg;
    msg.set_type(I_LEAVE);
    return std::make_unique<GameMessage>(msg);
}

std::unique_ptr<GameMessage> Builder::RoomList(std::vector<Room> rooms) {
    auto *rl = new class RoomList;
    for (const auto &room: rooms) {
        GameRoom *r = rl->add_rooms();
        r->set_name(room.name);
        r->set_playercount(room.players);
        r->set_maxplayercount(room.maxPlayers);
    }

    GameMessage msg;
    msg.set_type(ROOM_LIST);
    msg.set_allocated_roomlist(rl);
    return std::make_unique<GameMessage>(msg);
}

std::unique_ptr<GameMessage> Builder::WelcomeToRoom(std::vector<Player> players) {
    auto *wtr = new class WelcomeToRoom;
    for (const auto &player: players) {
        GamePlayer *gp = wtr->add_players();
        gp->set_username(player.username);
        gp->set_color(player.color);
    }

    GameMessage msg;
    msg.set_type(WELCOME_TO_ROOM);
    msg.set_allocated_welcometoroom(wtr);
    return std::make_unique<GameMessage>(msg);
}

std::unique_ptr<GameMessage> Builder::GameJoin(Player player) {
    auto *p = new class GamePlayer;
    p->set_username(player.username);
    p->set_color(player.color);

    auto *gj = new class GameJoin;
    gj->set_allocated_player(p);

    GameMessage msg;
    msg.set_type(GAME_JOIN);
    msg.set_allocated_gamejoin(gj);
    return std::make_unique<GameMessage>(msg);
}

std::unique_ptr<GameMessage> Builder::GameWait(int32_t waitingFor) {
    auto *gw = new class GameWait;
    gw->set_waitingfor(waitingFor);

    GameMessage msg;
    msg.set_type(GAME_WAIT);
    msg.set_allocated_gamewait(gw);
    return std::make_unique<GameMessage>(msg);
}

std::unique_ptr<GameMessage> Builder::GameStart() {
    GameMessage msg;
    msg.set_type(GAME_START);
    return std::make_unique<GameMessage>(msg);
}

std::unique_ptr<GameMessage> Builder::OtherBombPlace(const std::string &username, int64_t timestamp, float x, float y) {
    auto *obp = new class OtherBombPlace;
    obp->set_username(username);
    obp->set_timestamp(timestamp);
    obp->set_x(x);
    obp->set_y(y);

    GameMessage msg;
    msg.set_type(OTHER_BOMB_PLACE);
    msg.set_allocated_otherbombplace(obp);
    return std::make_unique<GameMessage>(msg);
}

std::unique_ptr<GameMessage> Builder::GotHit(const std::string &username, int32_t livesRemaining, int64_t timestamp) {
    auto *gh = new class GotHit;
    gh->set_username(username);
    gh->set_livesremaining(livesRemaining);
    gh->set_timestamp(timestamp);

    GameMessage msg;
    msg.set_type(GOT_HIT);
    msg.set_allocated_gothit(gh);
    return std::make_unique<GameMessage>(msg);
}

std::unique_ptr<GameMessage> Builder::OtherMove(const std::string &name, float x, float y) {
    auto *om = new class OtherMove;
    om->set_username(name);
    om->set_x(x);
    om->set_y(y);

    GameMessage msg;
    msg.set_type(OTHER_MOVE);
    msg.set_allocated_othermove(om);
    return std::make_unique<GameMessage>(msg);
}

std::unique_ptr<GameMessage> Builder::OtherLeave(const std::string &username) {
    auto *ol = new class OtherLeave;
    ol->set_username(username);

    GameMessage msg;
    msg.set_type(OTHER_LEAVE);
    msg.set_allocated_otherleave(ol);
    return std::make_unique<GameMessage>(msg);
}

std::unique_ptr<GameMessage> Builder::GameWon(const std::string &winnerUsername) {
    auto *gw = new class GameWon;
    gw->set_winnerusername(winnerUsername);

    GameMessage msg;
    msg.set_type(GAME_WON);
    msg.set_allocated_gamewon(gw);
    return std::make_unique<GameMessage>(msg);
}