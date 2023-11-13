#ifndef BOOMBERMAN_MESSAGE_H
#define BOOMBERMAN_MESSAGE_H

#include <string>
#include <optional>

enum MessageType {
    NONE, // TODO: This shouldn't exist
    GAMEJOIN,
    IMOVE,
    IPLACEBOMB,
    ILEAVE,
    GOTHIT,
    GAMEWAIT,
    OTHERBOMBPLACE,
    OTHERMOVE,
    OTHERLEAVE,
    GAMEWON,
};

class Message {
private:
    int author = 0;

public:
    [[nodiscard]] virtual std::string name() const { return "None"; };

    [[nodiscard]] virtual MessageType type() const { return NONE; };

    void setAuthor(int fd) { author = fd; };

    std::optional<int> getAuthor() {
        if (!author) return std::nullopt;
        return author;
    }

    virtual ~Message() = default;
};

#endif
