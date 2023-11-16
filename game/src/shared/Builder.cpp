#include "Builder.h"
#include "messages.pb.h"

std::unique_ptr<GameMessage> Builder::IPlaceBomb(float x, float y) {
    auto *ipb = new IPlaceBombMsg;
    ipb->set_x(x);
    ipb->set_y(y);
    GameMessage msg;
    msg.set_message_type(I_PLACE_BOMB);
    msg.set_allocated_i_place_bomb(ipb);
    return std::make_unique<GameMessage>(msg);
}

std::unique_ptr<GameMessage> Builder::IMove(float x, float y) {
    auto *im = new IMoveMsg;
    im->set_x(x);
    im->set_x(y);
    GameMessage msg;
    msg.set_message_type(I_MOVE);
    msg.set_allocated_i_move(im);
    return std::make_unique<GameMessage>(msg);
}

std::unique_ptr<GameMessage> Builder::ILeave() {
    auto *il = new ILeaveMsg;
    GameMessage msg;
    msg.set_message_type(I_LEAVE);
    msg.set_allocated_i_leave(il);
    return std::make_unique<GameMessage>(msg);
}

std::unique_ptr<GameMessage> Builder::GameWait(int32_t waitingFor) {
    auto *gw = new GameWaitMsg;
    gw->set_waitingfor(waitingFor);
    GameMessage msg;
    msg.set_message_type(GAME_WAIT);
    msg.set_allocated_game_wait(gw);
    return std::make_unique<GameMessage>(msg);
}

std::unique_ptr<GameMessage> Builder::GameStart() {
    auto *gs = new GameStartMsg;
    GameMessage msg;
    msg.set_message_type(GAME_START);
    msg.set_allocated_game_start(gs);
    return std::make_unique<GameMessage>(msg);
}

std::unique_ptr<GameMessage> Builder::OtherBombPlace(int64_t timestamp, const std::string &name, float x, float y) {
    auto *obp = new OtherBombPlaceMsg;
    obp->set_timestamp(timestamp);
    obp->set_name(name);
    obp->set_x(x);
    obp->set_y(y);
    GameMessage msg;
    msg.set_message_type(OTHER_BOMB_PLACE);
    msg.set_allocated_other_bomb_place(obp);
    return std::make_unique<GameMessage>(msg);
}

std::unique_ptr<GameMessage> Builder::GotHit(const std::string &name, int32_t livesRemaining) {
    auto *gh = new GotHitMsg;
    gh->set_name(name);
    gh->set_livesremaining(livesRemaining);
    GameMessage msg;
    msg.set_message_type(GOT_HIT);
    msg.set_allocated_got_hit(gh);
    return std::make_unique<GameMessage>(msg);
}

std::unique_ptr<GameMessage> Builder::OtherMove(const std::string &name, float x, float y) {
    auto *om = new OtherMoveMsg;
    om->set_name(name);
    om->set_x(x);
    om->set_y(y);
    GameMessage msg;
    msg.set_message_type(OTHER_MOVE);
    msg.set_allocated_other_move(om);
    return std::make_unique<GameMessage>(msg);
}

std::unique_ptr<GameMessage> Builder::OtherLeave(const std::string &name) {
    auto *ol = new OtherLeaveMsg;
    ol->set_name(name);
    GameMessage msg;
    msg.set_message_type(OTHER_LEAVE);
    msg.set_allocated_other_leave(ol);
    return std::make_unique<GameMessage>(msg);
}

std::unique_ptr<GameMessage> Builder::GameWon(const std::string &winner) {
    auto *gw = new GameWonMsg;
    gw->set_winner(winner);
    GameMessage msg;
    msg.set_message_type(GAME_WON);
    msg.set_allocated_game_won(gw);
    return std::make_unique<GameMessage>(msg);
}

std::unique_ptr<GameMessage> Builder::GameJoin(const std::string &name, Color color, bool you) {
    auto *gj = new GameJoinMsg;
    gj->set_name(name);
    gj->set_color(color);
    gj->set_you(you);
    GameMessage msg;
    msg.set_message_type(GAME_JOIN);
    msg.set_allocated_game_join(gj);
    return std::make_unique<GameMessage>(msg);
}

std::unique_ptr<GameMessage> Builder::GameError(const std::string &error) {
    auto *e = new ErrorMsg;
    e->set_error(error);
    GameMessage msg;
    msg.set_message_type(GAME_ERROR);
    msg.set_allocated_error(e);
    return std::make_unique<GameMessage>(msg);
}

std::unique_ptr<LobbyMessage> Builder::GetRoomList() {
    auto *grl = new GetRoomListMsg;
    LobbyMessage msg;
    msg.set_message_type(GET_ROOM_LIST);
    msg.set_allocated_get_available_rooms(grl);
    return std::make_unique<LobbyMessage>(msg);
}

std::unique_ptr<LobbyMessage> Builder::RoomList(std::vector<Room> rooms) {
    auto *rl = new RoomListMsg;
    for (const auto &room: rooms) {
        RoomMsg *r = rl->add_rooms();
        r->set_name(room.name);
        r->set_players(room.players);
        r->set_maxplayers(room.maxPlayers);
    }

    LobbyMessage msg;
    msg.set_message_type(ROOM_LIST);
    msg.set_allocated_available_rooms(rl);
    return std::make_unique<LobbyMessage>(msg);
}

std::unique_ptr<LobbyMessage> Builder::JoinRoom(const std::string &name, std::optional<Room> room) {
    auto *jr = new JoinRoomMsg;
    jr->set_username(name);
    if (room.has_value()) {
        auto *r = new RoomMsg;
        r->set_name(room->name);
        r->set_players(room->players);
        r->set_maxplayers(room->maxPlayers);
        jr->set_allocated_room(r);
    }
    LobbyMessage msg;
    msg.set_message_type(JOIN_ROOM);
    msg.set_allocated_join_room(jr);
    return std::make_unique<LobbyMessage>(msg);
}

std::unique_ptr<LobbyMessage> Builder::LobbyError(const std::string &error) {
    auto *e = new ErrorMsg;
    e->set_error(error);
    LobbyMessage msg;
    msg.set_message_type(LOBBY_ERROR);
    msg.set_allocated_error(e);
    return std::make_unique<LobbyMessage>(msg);
}