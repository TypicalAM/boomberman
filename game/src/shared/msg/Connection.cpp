#include <csignal>
#include "Connection.h"

std::optional<int> Connection::Send(std::unique_ptr<GameMessage> msg) {
    char buf[256];
    size_t msg_size = msg->ByteSizeLong();
    buf[0] = static_cast<unsigned int>(msg_size);
    msg->SerializeToArray(buf + 1, msg_size);
    size_t bytes_sent = write(sock, buf, msg_size + 1);
    if (bytes_sent != msg_size + 1) {
        std::cerr << "[Connection] Couldn't send enough data, sent " << bytes_sent << "/" << msg_size + 1 << std::endl;
        size_t total_sent = bytes_sent;
        while (total_sent != msg_size + 1) {
            bytes_sent = write(sock, buf + total_sent, msg_size + 1 - total_sent);
            if (bytes_sent <= 0) return std::nullopt;
            total_sent += bytes_sent;
        }

        return total_sent;
    }

    return bytes_sent;
}

std::optional<std::unique_ptr<GameMessage>> Connection::Receive() {
    // The queue first
    if (!inboundQueue.empty()) {
        auto msg = std::move(inboundQueue.front());
        inboundQueue.pop();
        return msg;
    }

    char buf[256];
    int bytes_received = read(sock, buf, 256);
    if (bytes_received <= 0) return std::nullopt;
    uint8_t msg_size = static_cast<unsigned char>(buf[0]);
    if (bytes_received == msg_size + 1) {
        // Everything is fine, let's deserialize and return
        GameMessage msg;
        msg.ParseFromArray(buf + 1, bytes_received);
        return std::make_unique<GameMessage>(msg);
    }

    if (bytes_received > msg_size + 1) {
        // We received more than one message (maybe one is partial)
        GameMessage msg;
        msg.ParseFromArray(buf + 1, msg_size);
        inboundQueue.push(std::make_unique<GameMessage>(msg));

        // Now we decrement the bytes received for the second message and recalculate
        // the size
        bytes_received -= msg_size + 1;
        memmove(buf, buf + msg_size + 1, 256 - msg_size - 1);
        msg_size = static_cast<unsigned char>(buf[0]);
    }

    // Read until we get the message
    uint8_t read_total = bytes_received;
    while (read_total != msg_size + 1) {
        bytes_received = read(sock, buf + read_total + 1, 256 - read_total - 1);
        if (bytes_received <= 0) return std::nullopt;
        read_total += bytes_received;
    }

    GameMessage msg;
    msg.ParseFromArray(buf + 1, msg_size);
    inboundQueue.push(std::make_unique<GameMessage>(msg));

    auto result = std::move(inboundQueue.front());
    inboundQueue.pop();
    return result;
}

Connection::Connection(int sock) {
    this->sock = sock;
}