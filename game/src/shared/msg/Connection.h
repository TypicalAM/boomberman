#ifndef BOOMBERMAN_CONNECTION_H
#define BOOMBERMAN_CONNECTION_H

#include <optional>
#include <memory>
#include <queue>
#include "../proto/messages.pb.h"
#include "../Util.h"

namespace Builder {
    struct Room {
        std::string name;
        int32_t players;
        int32_t maxPlayers;
    };

    struct Player {
        std::string username;
        PlayerColor color;
    };
}

// Send and receive messages from sockets
class Connection {
private:
    std::unique_ptr<GameMessage> msg;
    std::queue<std::unique_ptr<GameMessage>> inboundQueue;

    [[nodiscard]] std::optional<int> Send();

public:
    int sock;

    std::optional<std::unique_ptr<GameMessage>> Receive();

    std::optional<int> SendError(std::string error);

    std::optional<int> SendGetRoomList();

    std::optional<int> SendJoinRoom(const std::string &name, std::optional<std::string> roomName = std::nullopt);

    std::optional<int> SendIPlaceBomb(float x, float y);

    std::optional<int> SendIMove(float x, float y);

    std::optional<int> SendILeave();

    std::optional<int> SendRoomList(const std::vector<Builder::Room>& rooms);

    std::optional<int> SendWelcomeToRoom(const std::vector<Builder::Player>& players);

    std::optional<int> SendGameJoin(Builder::Player player);

    std::optional<int> SendGameStart();

    std::optional<int> SendOtherBombPlace(std::string username, Timestamp timestamp, float x, float y);

    std::optional<int> SendGotHit(const std::string &username, int32_t livesRemaining, Timestamp timestamp);

    std::optional<int> SendOtherMove(std::string name, float x, float y);

    std::optional<int> SendOtherLeave(const std::string &username);

    std::optional<int> SendGameWon(std::string winnerUsername);

    explicit Connection(int sock);
};

#endif