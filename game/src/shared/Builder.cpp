#include "Builder.h"
#include "messages.pb.h"

std::unique_ptr<GameMessage> Builder::IPlaceBomb(float x, float y) {
    IPlaceBombMsg ipb;
    ipb.set_x(x);
    ipb.set_y(y);
    GameMessage msg;
    msg.set_message_type(I_PLACE_BOMB);
    msg.set_allocated_i_place_bomb(&ipb);
    return std::make_unique<GameMessage>(msg);
}

std::unique_ptr<GameMessage> Builder::IMove(float x, float y) {
    IMoveMsg im;
    im.set_x(x);
    im.set_x(y);
    GameMessage msg;
    msg.set_message_type(I_MOVE);
    msg.set_allocated_i_move(&im);
    return std::make_unique<GameMessage>(msg);
}

std::unique_ptr<GameMessage> Builder::ILeave() {
    ILeaveMsg il;
    GameMessage msg;
    msg.set_message_type(I_LEAVE);
    msg.set_allocated_i_leave(&il);
    return std::make_unique<GameMessage>(msg);
}

std::unique_ptr<GameMessage> Builder::GameWait(int32_t waitingFor) {
    GameWaitMsg gw;
    gw.set_waitingfor(waitingFor);
    GameMessage msg;
    msg.set_message_type(GAME_WAIT);
    msg.set_allocated_game_wait(&gw);
    return std::make_unique<GameMessage>(msg);
}

std::unique_ptr<GameMessage> Builder::OtherBombPlace(int64_t timestamp, const std::string &name, float x, float y) {
    OtherBombPlaceMsg obp;
    obp.set_timestamp(timestamp);
    obp.set_name(name);
    obp.set_x(x);
    obp.set_y(y);
    GameMessage msg;
    msg.set_message_type(OTHER_BOMB_PLACE);
    msg.set_allocated_other_bomb_place(&obp);
    return std::make_unique<GameMessage>(msg);
}

std::unique_ptr<GameMessage> Builder::GotHitMessage(const std::string &name, int32_t livesRemaining) {
    GotHitMsg gh;
    gh.set_name(name);
    gh.set_livesremaining(livesRemaining);
    GameMessage msg;
    msg.set_message_type(GOT_HIT_MESSAGE);
    msg.set_allocated_got_hit_message(&gh); // TODO: WHy does that look like this
    return std::make_unique<GameMessage>(msg);
}

std::unique_ptr<GameMessage> Builder::OtherMove(const std::string &name, float x, float y) {
    OtherMoveMsg om;
    om.set_name(name);
    om.set_x(x);
    om.set_y(y);
    GameMessage msg;
    msg.set_message_type(OTHER_MOVE);
    msg.set_allocated_other_move(&om);
    return std::make_unique<GameMessage>(msg);
}

std::unique_ptr<GameMessage> Builder::OtherLeave(const std::string &name) {
    OtherLeaveMsg ol;
    ol.set_name(name);
    GameMessage msg;
    msg.set_message_type(OTHER_LEAVE);
    msg.set_allocated_other_leave(&ol);
    return std::make_unique<GameMessage>(msg);
}

std::unique_ptr<GameMessage> Builder::GameWon(const std::string &winner) {
    GameWonMsg gw;
    gw.set_winner(winner);
    GameMessage msg;
    msg.set_message_type(GAME_WON);
    msg.set_allocated_game_won(&gw);
    return std::make_unique<GameMessage>(msg);
}

std::unique_ptr<GameMessage> Builder::GameJoin(const std::string &name, Color color, bool you) {
    GameJoinMsg gj;
    gj.set_name(name);
    gj.set_color(color);
    gj.set_you(you);
    GameMessage msg;
    msg.set_message_type(GAME_JOIN);
    msg.set_allocated_game_join(&gj);
    return std::make_unique<GameMessage>(msg);
}