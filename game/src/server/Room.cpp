#include <utility>
#include <thread>
#include <sys/epoll.h>
#include <csignal>
#include <random>
#include <sys/socket.h>
#include "Room.h"
#include "../shared/msg/Builder.h"

void Room::GameLoop() {
    while (state.load() != GAME_OVER) {
        ReadIntoQueue();
        CheckIfGameReady();
        HandleQueue();
        HandleGameUpdates();
    }

    LOG << "Gameloop done";
}

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

void Room::CheckIfGameReady() {
    if (state.load() != WAIT_FOR_START) return;

    std::lock_guard<std::mutex> lock(playerMtx);
    if (MAX_PLAYERS - clientCount > 0) {
        if (Util::TimestampMillis() < lastGameWaitMessage + GAME_WAIT_MESSAGE_INTERVAL) return;
        SendBroadcast(Builder::GameWait, MAX_PLAYERS - clientCount);
        lastGameWaitMessage = Util::TimestampMillis();
        return;
    }

    LOG << "Starting game";
    state.store(PLAY);
    SendBroadcast(Builder::GameStart);
}

void Room::HandleGameUpdates() {
    if (state.load() != PLAY) return;

    std::lock_guard<std::mutex> lock(playerMtx);
    std::vector<int> explode_indices;

    for (int i = 0; i < bombs.size(); i++) {
        if (!bombs[i].ShouldExplode()) continue;
        explode_indices.push_back(i);

        // The bomb explodes
        std::vector<TileOnFire> result = bombs[i].boom(map.get());
        for (auto &player: players) {
            // Let's check if the player was in radius of the bomb
            int adjusted_x = std::floor(player->coords.x);
            int adjusted_y = std::floor(player->coords.y);
            for (const auto &tile: result) {
                if (adjusted_x == tile.x && adjusted_y == tile.y) {
                    // Check if we are in iframes
                    if (player->immunityEndTimestamp > Util::TimestampMillis()) {
                        LOG << player->username << "is immune since " << player->immunityEndTimestamp << " > " << Util::TimestampMillis();
                        continue;
                    }

                    // The person was hit by the bomb, cool
                    player->immunityEndTimestamp = Util::TimestampMillis() + IMMUNITY_TIME_MILLIS;
                    player->livesRemaining--;
                    SendBroadcast(Builder::GotHit, player->username, player->livesRemaining, Util::TimestampMillis());
                    std::cout << "Player: " << player->username << " got hit. Lives remaining: "
                              << player->livesRemaining << std::endl;
                }
            }
        }
    }

    for (const auto &index: explode_indices)
        bombs.erase(bombs.begin() + index);

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
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> distrib(0, explode_indices.size());
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
        close(epollSock);
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
        close(epollSock);
    }
}

void Room::HandleQueue() {
    std::lock_guard<std::mutex> lock(msgQueueMtx);
    while (!msgQueue.empty()) {
        HandleMessage(std::move(msgQueue.front()));
        msgQueue.pop();
    }
}

void Room::HandleMessage(std::unique_ptr<AuthoredMessage> msg) {
    LOG << "Handling a message from user: " << msg->author->username;

    // If the game started
    if (state.load() == WAIT_FOR_START) {
        if (msg->payload->type() != I_LEAVE) {
            SendSpecific(msg->author->conn.get(), Builder::Error("Game hasn't started yet"));
            return;
        }

        // Make the player leave
        shutdown(players[0]->conn->sock, SHUT_RDWR);
        close(players[0]->conn->sock);
        return;
    }

    // Check if the player is playing tricks
    if (!msg->author->livesRemaining && msg->payload->type() != I_LEAVE) {
        SendSpecific(msg->author->conn.get(), Builder::Error("You can't really do anything while dead, can you?"));
        return;
    }

    // Handle message based on the type
    switch (msg->payload->type()) {
        case I_PLACE_BOMB: {
            // Place the bomb at the specified location
            IPlaceBomb ipb = msg->payload->iplacebomb();
            int64_t timestamp = Util::TimestampMillis();
            bombs.emplace_back(std::floor(ipb.x()), std::floor(ipb.y()), 3, 25, Util::TimestampMillis(), 3.0f, false);
            SendExcept(msg->author->conn.get(), Builder::OtherBombPlace, msg->author->username, timestamp, ipb.x(),
                       ipb.y());
            return;
        }

        case I_MOVE: {
            // Check if the movement is valid and move the players character
            IMove im = msg->payload->imove();
            int tile_state = map->getSquareState(std::floor(im.x()), std::floor(im.y()));
            if (tile_state != NOTHIN) {
                SendSpecific(msg->author->conn.get(), Builder::Error("Invalid movement"));
                return;
            }
            // The movement was valid, let's roll
            msg->author->coords.x = im.x();
            msg->author->coords.y = im.y();
            SendExcept(msg->author->conn.get(), Builder::OtherMove, msg->author->username, im.x(), im.y());
            return;
        }

        case I_LEAVE: {
            // Notify others of the player leaving
            std::lock_guard<std::mutex> lock(playerMtx);
            if (players.size() == 1) {
                shutdown(players[0]->conn->sock, SHUT_RDWR);
                close(players[0]->conn->sock);
                state.store(GAME_OVER); // Close the game
                close(epollSock);
                return;
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
                close(epollSock);
                return;
            }

            SendExcept(msg->author->conn.get(), Builder::OtherLeave, msg->author->username);
            if (state.load() == PLAY) {
                auto coords = msg->author->coords;
                SendExcept(msg->author->conn.get(), Builder::OtherBombPlace, "Server", Util::TimestampMillis(),
                           coords.x, coords.y);
                bombs.emplace_back(std::floor(coords.x), std::floor(coords.y), 9, 25, Util::TimestampMillis(), 3.0f,
                                   true);
            }
            shutdown(msg->author->conn->sock, SHUT_RDWR);
            close(msg->author->conn->sock);
            int author_idx = -1;
            for (int i = 0; i < players.size(); ++i)
                if (players[i]->conn->sock == msg->author->conn->sock)
                    author_idx = i;
            players.erase(players.begin() + author_idx);
            return;
        }

        default:
            LOG << "Unexpected message type: " << msg->payload->type();
            SendSpecific(msg->author->conn.get(), Builder::Error("Unexpected message"));
    }
}

void Room::ReadIntoQueue() {
    if (state.load() == GAME_OVER) return;

    epoll_event events[10];
    int event_num = epoll_wait(epollSock, events, 10, 0);
    for (int i = 0; i < event_num; ++i) {
        // If this isn't an in event, ignore it
        if (!(events[i].events & EPOLLIN)) {
            LOG << "Ignoring event on socket: " << events[i].data.fd;
            continue;
        }

        // We got a message from a client
        std::lock_guard<std::mutex> lock(playerMtx);
        auto client_conn = static_cast<Connection *>(events[i].data.ptr);
        auto msg = client_conn->Receive();
        if (!msg.has_value()) {
            LOG << "Closing connection since we can't receive data: " << client_conn->sock;
            epoll_ctl(epollSock, EPOLL_CTL_DEL, client_conn->sock, nullptr);
            shutdown(client_conn->sock, SHUT_RDWR);
            close(client_conn->sock);
            clientCount--;

            int author_idx = -1;
            for (int j = 0; j < players.size(); ++j) if (players[j]->conn->sock == client_conn->sock) author_idx = j;
            if (author_idx == -1) continue; // Look below

            SendExcept(players[author_idx]->conn.get(), Builder::OtherLeave, players[author_idx]->username);
            if (state.load() == PLAY) {
                auto coords = players[author_idx]->coords;
                SendExcept(players[author_idx]->conn.get(), Builder::OtherBombPlace, "Server", Util::TimestampMillis(),
                           coords.x, coords.y);
                bombs.emplace_back(std::floor(coords.x), std::floor(coords.y), 7, 25, Util::TimestampMillis(), 3.0f,
                                   true);
            }
            players.erase(players.begin() + author_idx);
            continue;
        }

        // Find the player and associate it with the message
        int author_idx = -1;
        for (int j = 0; j < players.size(); ++j) if (players[j]->conn->sock == client_conn->sock) author_idx = j;
        if (author_idx == -1) {
            // This happens when a player has been disconnected from the game
            // (kicked out, idk) but they still somehow sent a message here. In that
            // case broadcasting of their leaving should've already been handled above.
            LOG << "Closing connection since there isn't a player like that: " << client_conn->sock;
            epoll_ctl(epollSock, EPOLL_CTL_DEL, client_conn->sock, nullptr);
            shutdown(client_conn->sock, SHUT_RDWR);
            close(client_conn->sock);
            continue;
        }

        // Push the authored message into the queue
        std::lock_guard<std::mutex> queue_lock(msgQueueMtx);
        msgQueue.push(
                std::make_unique<AuthoredMessage>(AuthoredMessage{std::move(msg.value()), players[author_idx]}));
    }
}

bool Room::JoinPlayer(std::unique_ptr<Connection> conn, const std::string &username) {
    if (state.load() != WAIT_FOR_START) return false;
    std::lock_guard<std::mutex> lock(playerMtx);

    // Insert the player at the first pos
    // TODO: Igorze, dlaczego muszę popychać całą tablicę do tyłu...
    // TODO: Adamie, zamykasz tabele, będziesz na czele...
    auto color = static_cast<PlayerColor>(players.size());
    players.insert(players.begin(), std::make_unique<SPlayer>(std::move(conn), username, color));

    // Send the current connected player list to the new player
    std::vector<Builder::Player> player_list;
    for (const auto &player: players) player_list.emplace_back(Builder::Player{player->username, player->color});

    SendSpecific(players[0]->conn.get(), Builder::WelcomeToRoom(player_list));
    SendExcept(players[0]->conn.get(), Builder::GameJoin, Builder::Player{username, color});

    epoll_event event = {EPOLLIN | EPOLLET, epoll_data{.ptr = players[0]->conn.get()}};
    if (epoll_ctl(epollSock, EPOLL_CTL_ADD, players[0]->conn->sock, &event) == -1)
        throw std::runtime_error("cannot add to epoll");
    clientCount++;
    return true;
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
    msgQueue = std::queue<std::unique_ptr<AuthoredMessage>>();
    lastGameWaitMessage = Util::TimestampMillis();

    // Create epoll instance
    if ((epollSock = epoll_create1(0)) == -1) {
        close(epollSock); // Clean up on failure
        throw std::runtime_error("epoll creation failed");
    }
}