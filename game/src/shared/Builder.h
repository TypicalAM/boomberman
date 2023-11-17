#ifndef BOOMBERMAN_BUILDER_H
#define BOOMBERMAN_BUILDER_H

// Methods for building various messages
#include <memory>
#include <optional>
#include "messages.pb.h"

namespace Builder {
    struct Room {
        std::string name;
        int32_t players;
        int32_t maxPlayers;
    };

    std::unique_ptr<GameMessage> IPlaceBomb(float x, float y);

    std::unique_ptr<GameMessage> IMove(float x, float y);

    std::unique_ptr<GameMessage> ILeave();

    std::unique_ptr<GameMessage> GameJoin(const std::string &name, Color color, bool you);

    std::unique_ptr<GameMessage> GameWait(int32_t waitingFor);

    std::unique_ptr<GameMessage> GameStart();

    std::unique_ptr<GameMessage> OtherBombPlace(int64_t timestamp, const std::string &name, float x, float y);

    std::unique_ptr<GameMessage> GotHit(const std::string &name, int32_t livesRemaining);

    std::unique_ptr<GameMessage> OtherMove(const std::string &name, float x, float y);

    std::unique_ptr<GameMessage> OtherLeave(const std::string &name);

    std::unique_ptr<GameMessage> GameWon(const std::string &winner);

    std::unique_ptr<GameMessage> Error(const std::string &error);

    std::unique_ptr<GameMessage> GetRoomList();

    std::unique_ptr<GameMessage> RoomList(std::vector<Room> rooms);

    std::unique_ptr<GameMessage> JoinRoom(const std::string &name, std::optional<Room> room = std::nullopt);
};

#endif