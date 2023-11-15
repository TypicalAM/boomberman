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
    msg.set_message_type(GOT_HIT_MESSAGE);
    msg.set_allocated_got_hit_message(gh); // TODO: WHy does that look like this
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

int64_t Builder::Timestamp() {
    return static_cast<int64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count());;
}
