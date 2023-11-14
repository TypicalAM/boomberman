#include <csignal>
#include "Channel.h"

std::optional<int> Channel::Send(int sock, std::unique_ptr<GameMessage> msg) {
    char buf[256];
    msg->SerializeToArray(buf, 256);
    int bytes_sent = write(sock, buf, 256);
    if (bytes_sent == -1) return std::nullopt;
    return bytes_sent;
}

std::optional<std::unique_ptr<GameMessage>> Channel::Receive(int sock) {
    char buf[256];
    int bytes_received = read(sock, buf, 256);
    if (bytes_received == -1) return std::nullopt;
    GameMessage msg;
    msg.ParseFromArray(buf, bytes_received);
    return std::make_unique<GameMessage>(msg);
}

std::unique_ptr<GameMessage> Builder::IPlaceBomb(float x, float y) {
    IPlaceBombMsg ipb;
    ipb.set_x(x);
    ipb.set_y(y);
    GameMessage msg;
    msg.set_message_type(MessageType::I_PLACE_BOMB);
    msg.set_allocated_i_place_bomb(&ipb);
    return std::make_unique<GameMessage>(msg);
}

std::unique_ptr<GameMessage> Builder::GameJoin(const std::string &name, Color color, bool you) {
    GameJoinMsg gj;
    gj.set_name(name);
    gj.set_color(color);
    gj.set_you(you);
    GameMessage msg;
    msg.set_message_type(MessageType::GAME_JOIN);
    msg.set_allocated_game_join(&gj);
    return std::make_unique<GameMessage>(msg);
}