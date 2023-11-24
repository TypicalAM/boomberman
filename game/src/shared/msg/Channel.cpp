#include <csignal>
#include "Channel.h"

std::optional<int> Channel::Send(int sock, std::unique_ptr<GameMessage> msg) {
    char buf[256];
    msg->SerializeToArray(buf, msg->ByteSizeLong());
    int bytes_sent = write(sock, buf, msg->ByteSizeLong());
    if (bytes_sent == -1) return std::nullopt;
    return bytes_sent;
}

std::optional<std::unique_ptr<GameMessage>> Channel::Receive(int sock) {
    char buf[256];
    int bytes_received = read(sock, buf, 256);
    if (bytes_received <= 0) return std::nullopt;
    GameMessage msg;
    msg.ParseFromArray(buf, bytes_received);
    return std::make_unique<GameMessage>(msg);
}