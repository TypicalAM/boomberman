#include <utility>
#include <thread>
#include <sys/epoll.h>
#include <csignal>
#include <random>
#include <sys/socket.h>
#include "Room.h"
#include "../shared/msg/Builder.h"

void Room::SendSpecific(Connection *conn, std::unique_ptr<GameMessage> msg) {
    LOG << "Sending message of type: " << msg->type() << " to player sock " << conn->sock;
    if (!conn->Send(std::move(msg)).has_value()) {
        shutdown(conn->sock, SHUT_RDWR);
        close(conn->sock);
    }
}

template<typename Function, typename ...Args>
void Room::SendExcept(Connection *conn, Function &&builderFunc, Args &&... builderArgs) {
    for (auto &player: players) {
        if (player->conn->sock == conn->sock) continue;
        auto msg = std::invoke(std::forward<Function>(builderFunc), std::forward<Args>(builderArgs)...);
        LOG << "Sending message of type: " << msg->type() << " to player sock " << player->conn->sock;
        if (!player->conn->Send(std::move(msg)).has_value()) {
            shutdown(player->conn->sock, SHUT_RDWR);
            close(player->conn->sock);
        }
    }
}

template<typename Function, typename ...Args>
void Room::SendBroadcast(Function &&builderFunc, Args &&... builderArgs) {
    for (auto &player: players) {
        auto msg = std::invoke(std::forward<Function>(builderFunc), std::forward<Args>(builderArgs)...);
        LOG << "Sending message of type: " << msg->type() << " to player sock " << player->conn->sock;
        if (!player->conn->Send(std::move(msg)).has_value()) {
            shutdown(player->conn->sock, SHUT_RDWR);
            close(player->conn->sock);
        }
    }
}

bool Room::CanJoin(const std::string &username) {
    std::lock_guard<std::mutex> lock(playerMtx);
    bool already_exists = false;
    for (const auto &player: players) if (player->username == username) already_exists = true;
    return !already_exists && MAX_PLAYERS - clientCount > 0;
}

bool Room::HandleMessage(std::unique_ptr<AuthoredMessage> msg) {
    LOG << "Handling a message from user: " << msg->author->username;

    // If the game started
    if (state.load() == WAIT_FOR_START) {
        if (msg->payload->type() != I_LEAVE) {
            SendSpecific(msg->author->conn, Builder::Error("Game hasn't started yet"));
            return false;
        }

        // Make the player leave
        shutdown(players[0]->conn->sock, SHUT_RDWR);
        close(players[0]->conn->sock);
        return false;
    }

    // Check if the player is playing tricks
    if (!msg->author->livesRemaining && msg->payload->type() != I_LEAVE) {
        SendSpecific(msg->author->conn, Builder::Error("You can't really do anything while dead, can you?"));
        return false;
    }

    // Handle message based on the type
    switch (msg->payload->type()) {
        case I_PLACE_BOMB: {
            // Place the bomb at the specified location
            IPlaceBomb ipb = msg->payload->iplacebomb();
            Timestamp timestamp = Util::TimestampMillis();
            bombs.emplace(std::floor(ipb.x()), std::floor(ipb.y()), 3, 25, Util::TimestampMillis(), 3.0f, false);
            SendExcept(msg->author->conn, Builder::OtherBombPlace, msg->author->username, timestamp, ipb.x(),
                       ipb.y());
            return true;
        }

        case I_MOVE: {
            // Check if the movement is valid and move the players character
            IMove im = msg->payload->imove();
            int tile_state = map->getSquareState(std::floor(im.x()), std::floor(im.y()));
            if (tile_state != NOTHIN) {
                SendSpecific(msg->author->conn, Builder::Error("Invalid movement"));
                return false;
            }
            // The movement was valid, let's roll
            msg->author->coords.x = im.x();
            msg->author->coords.y = im.y();
            SendExcept(msg->author->conn, Builder::OtherMove, msg->author->username, im.x(), im.y());
            return false;
        }

        case I_LEAVE: {
            // Notify others of the player leaving
            std::lock_guard<std::mutex> lock(playerMtx);
            if (players.size() == 1) {
                shutdown(players[0]->conn->sock, SHUT_RDWR);
                close(players[0]->conn->sock);
                state.store(GAME_OVER); // Close the game
                return false;
            }

            if (players.size() == 2 && state.load() == PLAY) {
                // This player just won lol, let's notify the last one standing
                auto winner_idx = (players[0]->conn->sock == msg->author->conn->sock) ? 1 : 0;
                LOG << "Player won by other disconnecting: " << players[winner_idx]->username;
                SendBroadcast(Builder::GameWon, players[winner_idx]->username);
                state.store(WAIT_FOR_END);
                std::this_thread::sleep_for(std::chrono::seconds(3));
                LOG << "Closing other connections and ending the game";
                for (auto &player: players) {
                    shutdown(player->conn->sock, SHUT_RDWR);
                    close(player->conn->sock);
                }
                state.store(GAME_OVER);
                return false;
            }

            PlaceSuperBomb(msg->author);

            int author_idx = -1;
            for (int i = 0; i < players.size(); ++i)
                if (players[i]->conn->sock == msg->author->conn->sock)
                    author_idx = i;
            players.erase(players.begin() + author_idx);
            // TODO: Cleaner
            return true;
        }

        default:
            LOG << "Unexpected message type: " << msg->payload->type();
            SendSpecific(msg->author->conn, Builder::Error("Unexpected message"));
            return false;
    }
}

SPlayer *Room::JoinPlayer(Connection *conn, const std::string &username) {
    if (state.load() != WAIT_FOR_START) return nullptr; // TODO: This should never happen
    std::lock_guard<std::mutex> lock(playerMtx);

    // Insert the player at the first pos
    auto color = static_cast<PlayerColor>(players.size());
    players.insert(players.begin(), std::make_unique<SPlayer>(conn, username, color));
    clientCount++;

    // Send the current connected player list to the new player
    std::vector<Builder::Player> player_list;
    for (const auto &player: players) player_list.emplace_back(Builder::Player{player->username, player->color});
    SendSpecific(players[0]->conn, Builder::WelcomeToRoom(player_list));
    SendExcept(players[0]->conn, Builder::GameJoin, Builder::Player{username, color});

    if (MAX_PLAYERS - clientCount == 0) {
        LOG << "Starting game";
        state.store(PLAY);
        SendBroadcast(Builder::GameStart);
    }

    return players[0].get();
}

int Room::Players() {
    std::lock_guard<std::mutex> lock(playerMtx);
    return clientCount;
}

bool Room::IsGameOver() {
    return state.load() == GAME_OVER;
}

Room::Room(boost::log::sources::logger roomLogger) {
    logger = std::move(roomLogger);
    map = std::make_unique<Map>(25, MAP_WIDTH, MAP_HEIGHT); // TODO: This should just be constant???
    lastGameWaitMessage = Util::TimestampMillis();
}

void Room::PlaceSuperBomb(SPlayer *player) {
    SendExcept(player->conn, Builder::OtherLeave, player->username);
    if (state.load() == PLAY) {
        auto coords = player->coords;
        SendExcept(player->conn, Builder::OtherBombPlace, "Server", Util::TimestampMillis(), coords.x, coords.y);
        bombs.emplace(std::floor(coords.x), std::floor(coords.y), 9, 25, Util::TimestampMillis(), 3.0f, true);
    }
}

void Room::ExplodeBomb() {
    if (state.load() != PLAY) return;

    Bomb bomb = bombs.front();
    bombs.pop();
    if (bomb.ShouldExplode()) return; // TODO: This shouldn't happen, cuz we are notified by a timer

    // The bomb explodes
    std::vector<TileOnFire> result = bomb.boom(map.get());
    std::lock_guard<std::mutex> lock(playerMtx);
    for (auto &player: players) {
        // Let's check if the player was in radius of the bomb
        int adjusted_x = std::floor(player->coords.x);
        int adjusted_y = std::floor(player->coords.y);
        for (const auto &tile: result) {
            if (adjusted_x == tile.x && adjusted_y == tile.y) {
                // Check if we are in iframes
                if (player->immunityEndTimestamp > Util::TimestampMillis()) {
                    LOG << player->username << "is immune since " << player->immunityEndTimestamp << " > "
                        << Util::TimestampMillis();
                    continue;
                }

                // The person was hit by the bomb, cool
                player->immunityEndTimestamp = Util::TimestampMillis() + IMMUNITY_TIME_MILLIS;
                player->livesRemaining--;
                SendBroadcast(Builder::GotHit, player->username, player->livesRemaining, Util::TimestampMillis());
                LOG << "Player: " << player->username << " got hit. Lives remaining: " << player->livesRemaining;
            }
        }
    }

    int people_alive = 0;
    int last_alive_index = -1;
    for (int i = 0; i < players.size(); ++i) {
        if (players[i]->livesRemaining != 0) {
            people_alive++;
            last_alive_index = i;
        }
    }

    if (people_alive == 0 && state.load() == PLAY) {
        // This can happen in the unfortunate event that the last two players die from the same bomb. In this case we select the
        // winner randomly (I don't think the person who set the bomb deserves to win) and also it would require a message rewrite,
        // so I ain't doing it.
        // TODO: I don't think it does what I think, too late evening, brain not worky
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> distrib(0, people_alive);
        int winner_idx = distrib(gen);

        // This person has won the game, announce that and disconnect all clients
        LOG << "Player won by random mutual explosion: " << players[winner_idx]->username;
        SendBroadcast(Builder::GameWon, players[winner_idx]->username);
        state.store(WAIT_FOR_END);
        std::this_thread::sleep_for(std::chrono::seconds(3));
        LOG << "Closing other connections and ending the game";
        for (auto &player: players) {
            shutdown(player->conn->sock, SHUT_RDWR);
            close(player->conn->sock);
        }
        state.store(GAME_OVER);
    }

    if (people_alive == 1 && state.load() == PLAY) {
        // This person has won the game, announce that and disconnect all clients
        LOG << "Player won by last person alive: " << players[last_alive_index]->username;
        SendBroadcast(Builder::GameWon, players[last_alive_index]->username);
        state.store(WAIT_FOR_END);
        std::this_thread::sleep_for(std::chrono::seconds(3));
        LOG << "Closing other connections and ending the game";
        for (auto &player: players) {
            shutdown(player->conn->sock, SHUT_RDWR);
            close(player->conn->sock);
        }
        state.store(GAME_OVER);
    }
}
