#include "Room.h"
#include <random>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <thread>
#include <utility>

template <typename Function, typename... Args>
void Room::SendSpecific(SPlayer *player, Function &&builderFunc,
                        Args &&...builderArgs) {
  std::optional<int> bytes_sent =
      std::invoke(std::forward<Function>(builderFunc), player->conn,
                  std::forward<Args>(builderArgs)...);
  if (!bytes_sent.has_value())
    player->marked_for_disconnect = true;
}

template <typename Function, typename... Args>
void Room::SendExcept(SPlayer *player, Function &&builderFunc,
                      Args &&...builderArgs) {
  for (auto &game_player : players)
    if (game_player.get() != player)
      SendSpecific(game_player.get(), builderFunc, builderArgs...);
}

template <typename Function, typename... Args>
void Room::SendBroadcast(Function &&builderFunc, Args &&...builderArgs) {
  for (auto &player : players)
    SendSpecific(player.get(), builderFunc, builderArgs...);
}

bool Room::CanJoin(const std::string &username) {
  if (username.empty())
    return false;

  std::lock_guard<std::mutex> lock(playerMtx);
  bool already_exists = false;
  for (const auto &player : players)
    if (player->username == username)
      already_exists = true;
  return !already_exists && MAX_PLAYERS - clientCount > 0;
}

bool Room::HandleMessage(std::unique_ptr<AuthoredMessage> msg) {
  LOG << "Handling a message from user: " << msg->author->username;

  // If the game started
  if (state.load() == WAIT_FOR_START) {
    if (msg->payload->type() != I_LEAVE) {
      SendSpecific(msg->author, &Connection::SendError,
                   "Game hasn't started yet");
      return false;
    }

    // Make the player leave
    players[0]->marked_for_disconnect = true;
    return false;
  }

  // Check if the player is playing tricks
  if (!msg->author->livesRemaining && msg->payload->type() != I_LEAVE) {
    SendSpecific(msg->author, &Connection::SendError,
                 "You can't really do anything while dead, can you?");
    return false;
  }

  // Handle message based on the type
  switch (msg->payload->type()) {
  case I_PLACE_BOMB: {
    // Place the bomb at the specified location
    IPlaceBomb ipb = msg->payload->iplacebomb();
    Timestamp ts = Util::TimestampMillis();
    bombs.emplace(std::floor(ipb.x()), std::floor(ipb.y()), 3, 25,
                  Util::TimestampMillis(), 3.0f, false);
    SendExcept(msg->author, &Connection::SendOtherBombPlace,
               msg->author->username, ts, ipb.x(), ipb.y());
    return true;
  }

  case I_MOVE: {
    // Check if the movement is valid and move the players character
    IMove im = msg->payload->imove();
    int tile_state =
        map->getSquareState(std::floor(im.x()), std::floor(im.y()));
    if (tile_state != NOTHIN) {
      SendSpecific(msg->author, &Connection::SendError, "Invalid movement");
      return false;
    }

    // The movement was valid, let's roll
    msg->author->coords.x = im.x();
    msg->author->coords.y = im.y();
    SendExcept(msg->author, &Connection::SendOtherMove, msg->author->username,
               im.x(), im.y());
    return false;
  }

  case I_LEAVE: {
    // Notify others of the player leaving
    msg->author->marked_for_disconnect = true;
    return false;
  }

  default:
    LOG << "Unexpected message type: " << msg->payload->type();
    SendSpecific(msg->author, &Connection::SendError, "Unexpected message");
    return false;
  }
}

SPlayer *Room::JoinPlayer(Connection *conn, const std::string &username) {
  if (state.load() != WAIT_FOR_START)
    return nullptr;
  std::lock_guard<std::mutex> lock(playerMtx);

  // Insert the player at the first pos
  auto color = static_cast<PlayerColor>(players.size());
  players.insert(players.begin(),
                 std::make_unique<SPlayer>(conn, username, color));
  clientCount++;

  // Send the current connected player list to the new player
  std::vector<Builder::Player> player_list;
  for (const auto &player : players)
    player_list.emplace_back(Builder::Player{player->username, player->color});
  SendSpecific(players[0].get(), &Connection::SendWelcomeToRoom, player_list);
  SendExcept(players[0].get(), &Connection::SendGameJoin,
             Builder::Player{username, color});

  if (MAX_PLAYERS - clientCount == 0) {
    LOG << "Starting game";
    state.store(PLAY);
    SendBroadcast(&Connection::SendGameStart);
  }

  return players[0].get();
}

int Room::PlayerCount() {
  std::lock_guard<std::mutex> lock(playerMtx);
  return clientCount;
}

PlayerDestructionInfo Room::DisconnectPlayers() {
  int bombs_to_place = 0;
  std::vector<int> sockets_to_delete;

  std::lock_guard<std::mutex> lock(playerMtx);
  std::vector<SPlayer *> players_to_destroy;
  for (const auto &player : players)
    if (player->marked_for_disconnect) {
      SendExcept(player.get(), &Connection::SendOtherLeave, player->username);
      epoll_ctl(epollSock, EPOLL_CTL_DEL, player->conn->sock, nullptr);
      shutdown(player->conn->sock, SHUT_RDWR);
      close(player->conn->sock);
      sockets_to_delete.push_back(player->conn->sock);
      players_to_destroy.push_back(player.get());

      if (state.load() == PLAY && player->livesRemaining != 0) {
        bombs_to_place++;
        int x = std::floor(player->coords.x);
        int y = std::floor(player->coords.y);
        bombs.emplace(x, y, 9, 25, Util::TimestampMillis(), 3.0f, true);
        SendExcept(player.get(), &Connection::SendOtherBombPlace, "Server",
                   Util::TimestampMillis(), x, y);
      }
    }

  // Erase elements at specified indices
  for (auto to_destroy : players_to_destroy) {
    for (int i = 0; i < players.size(); i++)
      if (players[i].get() == to_destroy) {
        LOG << "Disconnecting player " << players[i]->username;
        players.erase(players.begin() + i);
        break;
      }
  }

  switch (players.size()) {
  case 0:
    state.store(GAME_OVER);
    return std::make_pair(bombs_to_place, sockets_to_delete);

  case 1:
    SendBroadcast(&Connection::SendGameWon, players[0]->username);
    state.store(WAIT_FOR_END);
    std::this_thread::sleep_for(std::chrono::seconds(3));
    LOG << "Closing other connections and ending the game";
    epoll_ctl(epollSock, EPOLL_CTL_DEL, players[0]->conn->sock, nullptr);
    shutdown(players[0]->conn->sock, SHUT_RDWR);
    close(players[0]->conn->sock);
    sockets_to_delete.push_back(players[0]->conn->sock);
    players.erase(players.begin());
    state.store(GAME_OVER);
    return std::make_pair(bombs_to_place, sockets_to_delete);

  default:
    return std::make_pair(bombs_to_place, sockets_to_delete);
  }
}

bool Room::IsGameOver() { return state.load() == GAME_OVER; }

Room::Room(boost::log::sources::logger roomLogger, int epollSock) {
  this->epollSock = epollSock;
  logger = std::move(roomLogger);
  map = std::make_unique<Map>(25, MAP_WIDTH, MAP_HEIGHT);
}

void Room::NotifyExplosion() {
  if (state.load() != PLAY || bombs.empty())
    return;

  Bomb bomb = bombs.front();
  bombs.pop();

  int people_alive_before = 0;
  int last_alive_index_before = -1;
  std::vector<int> alive_before_explosion;
  for (int i = 0; i < players.size(); ++i) {
    if (players[i]->livesRemaining != 0) {
      people_alive_before++;
      last_alive_index_before = i;
      alive_before_explosion.push_back(i);
    }
  }

  // The bomb explodes
  std::vector<TileOnFire> result = bomb.boom(map.get());
  std::lock_guard<std::mutex> lock(playerMtx);
  for (auto &player : players) {
    // Let's check if the player was in radius of the bomb
    int adjusted_x = std::floor(player->coords.x);
    int adjusted_y = std::floor(player->coords.y);
    for (const auto &tile : result) {
      if (adjusted_x == tile.x && adjusted_y == tile.y) {
        // Check if we are in iframes
        if (player->immunityEndTimestamp > Util::TimestampMillis()) {
          LOG << player->username << "is immune since "
              << player->immunityEndTimestamp << " > "
              << Util::TimestampMillis();
          continue;
        }

        // The person was hit by the bomb, cool
        player->immunityEndTimestamp =
            Util::TimestampMillis() + IMMUNITY_TIME_MILLIS;
        player->livesRemaining--;
        SendBroadcast(&Connection::SendGotHit, player->username,
                      player->livesRemaining, Util::TimestampMillis());
        LOG << "Player: " << player->username
            << " got hit. Lives remaining: " << player->livesRemaining;
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
    // This can happen in the unfortunate event that the last two players die
    // from the same bomb. In this case we select the person randomly (I don't
    // think the person who set the bomb deserves to win) and also it would
    // require a message rewrite, so I ain't doing it.
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, people_alive_before);
    int winner_idx = alive_before_explosion[distrib(gen)];

    // This person has won the game, announce that and disconnect all clients
    LOG << "Player won by random mutual explosion: "
        << players[winner_idx]->username;
    SendBroadcast(&Connection::SendGameWon, players[winner_idx]->username);
    state.store(WAIT_FOR_END);
    std::this_thread::sleep_for(std::chrono::seconds(3));
    LOG << "Closing other connections and ending the game";
    for (auto &player : players)
      player->marked_for_disconnect = true;
    state.store(GAME_OVER);
  }

  if (people_alive == 1 && state.load() == PLAY) {
    // This person has won the game, announce that and disconnect all clients
    LOG << "Player won by last person alive: "
        << players[last_alive_index]->username;
    SendBroadcast(&Connection::SendGameWon,
                  players[last_alive_index]->username);
    state.store(WAIT_FOR_END);
    std::this_thread::sleep_for(std::chrono::seconds(3));
    LOG << "Closing other connections and ending the game";
    for (auto &player : players)
      player->marked_for_disconnect = true;
    state.store(GAME_OVER);
  }
}
