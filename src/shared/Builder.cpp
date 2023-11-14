#include "Builder.h"
#include "messages.pb.h"

std::unique_ptr <GameMessage> Builder::IPlaceBomb(float x, float y) {
    IPlaceBombMsg ipb;
    ipb.set_x(x);
    ipb.set_y(y);
    GameMessage msg;
    msg.set_message_type(MessageType::I_PLACE_BOMB);
    msg.set_allocated_i_place_bomb(&ipb);
    return std::make_unique<GameMessage>(msg);
}

std::unique_ptr <GameMessage> Builder::GameJoin(const std::string &name, Color color, bool you) {
    GameJoinMsg gj;
    gj.set_name(name);
    gj.set_color(color);
    gj.set_you(you);
    GameMessage msg;
    msg.set_message_type(MessageType::GAME_JOIN);
    msg.set_allocated_game_join(&gj);
    return std::make_unique<GameMessage>(msg);
}