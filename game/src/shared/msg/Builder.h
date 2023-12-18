#ifndef BOOMBERMAN_BUILDER_H
#define BOOMBERMAN_BUILDER_H

#include <memory>
#include <optional>
#include "../proto/messages.pb.h"

// Methods for building various messages
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

    std::unique_ptr<GameMessage> Error(const std::string &error);

    std::unique_ptr<GameMessage> GetRoomList();

    std::unique_ptr<GameMessage> JoinRoom(const std::string &name, std::optional<std::string> roomName = std::nullopt);

    std::unique_ptr<GameMessage> IPlaceBomb(float x, float y);

    std::unique_ptr<GameMessage> IMove(float x, float y);

    std::unique_ptr<GameMessage> ILeave();

    std::unique_ptr<GameMessage> RoomList(std::vector<Room> rooms);

    std::unique_ptr<GameMessage> WelcomeToRoom(std::vector<Player> players);

    std::unique_ptr<GameMessage> GameJoin(Player player);

    std::unique_ptr<GameMessage> GameStart();

    std::unique_ptr<GameMessage> OtherBombPlace(const std::string &username, int64_t timestamp, float x, float y);

    std::unique_ptr<GameMessage> GotHit(const std::string &username, int32_t livesRemaining, int64_t timestamp);

    std::unique_ptr<GameMessage> OtherMove(const std::string &name, float x, float y);

    std::unique_ptr<GameMessage> OtherLeave(const std::string &username);

    std::unique_ptr<GameMessage> GameWon(const std::string &winnerUsername);
};

#endif