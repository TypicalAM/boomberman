#include "Room.h"
#include <optional>
#include <random>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <thread>
#include <utility>

Room::Room(boost::log::sources::logger roomLogger, int epollSock) {
  this->epollSock = epollSock;
  logger = std::move(roomLogger);
  map = std::make_unique<Map>(25, MAP_WIDTH, MAP_HEIGHT);
}

int Room::PlayerCount() {
  std::lock_guard<std::mutex> lock(playerMtx);
  return clientCount;
}

std::optional<std::string> Room::CanJoin(const std::string &username) {
  if (username.empty())
    return "Empty username";

  std::lock_guard<std::mutex> lock(playerMtx);
  bool already_exists = false;
  for (const auto &player : players)
    if (player->username == username)
      already_exists = true;

  if (already_exists)
    return "There already is a nick like that";

  if (MAX_PLAYERS - clientCount == 0)
    return "Room is full";

  return std::nullopt;
}

Player *Room::JoinPlayer(Connection *conn, const std::string &username) {
  if (state.load() != WAIT_FOR_START)
    return nullptr;
  std::lock_guard<std::mutex> lock(playerMtx);

  // Insert the player at the first pos
  auto color = static_cast<PlayerColor>(players.size());
  players.insert(players.begin(),
                 std::make_unique<Player>(conn, username, color));
  clientCount++;

  // Send the current connected player list to the new player
  std::vector<builder::Player> player_list;
  for (const auto &player : players)
    player_list.emplace_back(builder::Player{player->username, player->color});
  sendSpecific(players[0].get(), &Connection::SendWelcomeToRoom, player_list);
  sendExcept(players[0].get(), &Connection::SendGameJoin,
             builder::Player{username, color});

  if (MAX_PLAYERS - clientCount == 0) {
    LOG << "Starting game";
    state.store(PLAY);
    sendBroadcast(&Connection::SendGameStart);
  }

  return players[0].get();
}

bool Room::IsGameOver() { return state.load() == GAME_OVER; }

bool Room::HandleMessage(std::unique_ptr<AuthoredMessage> msg) {
  LOG << "Handling a message from user: " << msg->author->username;

  // If the game started
  if (state.load() == WAIT_FOR_START) {
    if (msg->payload->type() != I_LEAVE) {
      sendSpecific(msg->author, &Connection::SendError,
                   "Game hasn't started yet");
      return false;
    }

    // Make the player leave
    players[0]->markedForDisconnect = true;
    return false;
  }

  // Check if the player is playing tricks
  if (!msg->author->livesRemaining && msg->payload->type() != I_LEAVE) {
    sendSpecific(msg->author, &Connection::SendError,
                 "You can't really do anything while dead, can you?");
    return false;
  }

  // Handle message based on the type
  switch (msg->payload->type()) {
  case I_PLACE_BOMB: {
    // Place the bomb at the specified location
    IPlaceBomb ipb = msg->payload->iplacebomb();
    Timestamp ts = util::TimestampMillis();
    bombs.emplace(std::floor(ipb.x()), std::floor(ipb.y()), 3, 25,
                  util::TimestampMillis(), 3.0f, false);
    sendExcept(msg->author, &Connection::SendOtherBombPlace,
               msg->author->username, ts, ipb.x(), ipb.y());
    return true;
  }

  case I_MOVE: {
    // Check if the movement is valid and move the players character
    IMove im = msg->payload->imove();
    int adjusted_x = std::floor(im.x());
    int adjusted_y = std::floor(im.y());
    int tile_state = map->getSquareState(adjusted_x, adjusted_y);
    if (tile_state != NOTHIN) {
      LOG << "Correcting movement for " << msg->author->username;
      sendSpecific(msg->author, &Connection::SendMovementCorrection,
                   msg->author->GetCoords().x, msg->author->GetCoords().y);
      return false;
    }

    // Double check the movement
    std::optional<Coords> correction =
        msg->author->MoveCheckSus(adjusted_x, adjusted_y);
    if (correction.has_value()) {
      LOG << "Correcting movement for " << msg->author->username;
      sendSpecific(msg->author, &Connection::SendMovementCorrection,
                   correction->x, correction->y);
      return false;
    }

    sendExcept(msg->author, &Connection::SendOtherMove, msg->author->username,
               adjusted_x, adjusted_y);
    return false;
  }

  case I_LEAVE: {
    // Notify others of the player leaving
    msg->author->markedForDisconnect = true;
    return false;
  }

  default:
    LOG << "Unexpected message type: " << msg->payload->type();
    sendSpecific(msg->author, &Connection::SendError, "Unexpected message");
    return false;
  }
}

void Room::NotifyExplosion() {
  if (state.load() != PLAY || bombs.empty())
    return;

  Bomb bomb = bombs.front();
  bombs.pop();

  int people_alive_before = 0;
  std::vector<int> alive_before_explosion;
  for (int i = 0; i < players.size(); i++)
    if (players[i]->livesRemaining > 0) {
      people_alive_before++;
      alive_before_explosion.push_back(i);
    }

  // The bomb explodes
  std::vector<TileOnFire> result = bomb.boom(map.get());
  std::lock_guard<std::mutex> lock(playerMtx);
  for (auto &player : players) {
    // Let's check if the player was in radius of the bomb
    int adjusted_x = std::floor(player->GetCoords().x);
    int adjusted_y = std::floor(player->GetCoords().y);
    for (const auto &tile : result) {
      if (adjusted_x == tile.x && adjusted_y == tile.y &&
          player->livesRemaining > 0) {
        // Check if we are in iframes
        if (player->immunityEndTimestamp > util::TimestampMillis()) {
          LOG << player->username << " is immune since "
              << player->immunityEndTimestamp << " > "
              << util::TimestampMillis();
          continue;
        }

        // The person was hit by the bomb, cool
        player->immunityEndTimestamp =
            util::TimestampMillis() + IMMUNITY_TIME_MILLIS;
        player->livesRemaining--;
        sendBroadcast(&Connection::SendGotHit, player->username,
                      player->livesRemaining, util::TimestampMillis());
        LOG << "Player: " << player->username
            << " got hit. Lives remaining: " << player->livesRemaining;
      }
    }
  }

  int people_alive = 0;
  int last_alive_index = -1;
  for (int i = 0; i < players.size(); i++)
    if (players[i]->livesRemaining > 0) {
      people_alive++;
      last_alive_index = i;
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
    sendBroadcast(&Connection::SendGameWon, players[winner_idx]->username);
    state.store(WAIT_FOR_END);
    std::this_thread::sleep_for(std::chrono::seconds(3));
    LOG << "Closing other connections and ending the game";
    for (auto &player : players)
      player->markedForDisconnect = true;
    state.store(GAME_OVER);
  }

  if (people_alive == 1 && state.load() == PLAY) {
    // This person has won the game, announce that and disconnect all clients
    LOG << "Player won by last person alive: "
        << players[last_alive_index]->username;
    sendBroadcast(&Connection::SendGameWon,
                  players[last_alive_index]->username);
    state.store(WAIT_FOR_END);
    std::this_thread::sleep_for(std::chrono::seconds(3));
    LOG << "Closing other connections and ending the game";
    for (auto &player : players)
      player->markedForDisconnect = true;
    state.store(GAME_OVER);
  }
}

PlayerDestructionInfo Room::DisconnectPlayers() {
  int bombs_to_place = 0;
  std::vector<int> sockets_to_delete;

  std::lock_guard<std::mutex> lock(playerMtx);
  std::vector<Player *> players_to_destroy;
  for (const auto &player : players)
    if (player->markedForDisconnect) {
      sendExcept(player.get(), &Connection::SendOtherLeave, player->username);
      epoll_ctl(epollSock, EPOLL_CTL_DEL, player->conn->sock, nullptr);
      shutdown(player->conn->sock, SHUT_RDWR);
      close(player->conn->sock);
      sockets_to_delete.push_back(player->conn->sock);
      players_to_destroy.push_back(player.get());

      if (state.load() == PLAY && player->livesRemaining != 0) {
        bombs_to_place++;
        int x = std::floor(player->GetCoords().x);
        int y = std::floor(player->GetCoords().y);
        bombs.emplace(x, y, 9, 25, util::TimestampMillis(), 3.0f, true);
        sendExcept(player.get(), &Connection::SendOtherBombPlace, "Server",
                   util::TimestampMillis(), x, y);
      }
    }

  // Erase elements at specified indices
  for (auto to_destroy : players_to_destroy)
    for (int i = 0; i < players.size(); i++)
      if (players[i].get() == to_destroy) {
        LOG << "Disconnecting player " << players[i]->username;
        players.erase(players.begin() + i);
        break;
      }

  switch (players.size()) {
  case 0:
    state.store(GAME_OVER);
    return std::make_pair(bombs_to_place, sockets_to_delete);

  case 1:
    LOG << "Player won by last person not-disconnected: "
        << players[0]->username;
    sendBroadcast(&Connection::SendGameWon, players[0]->username);
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

template <typename Function, typename... Args>
void Room::sendSpecific(Player *player, Function &&builderFunc,
                        Args &&...builderArgs) {
  std::optional<int> bytes_sent =
      std::invoke(std::forward<Function>(builderFunc), player->conn,
                  std::forward<Args>(builderArgs)...);
  if (!bytes_sent.has_value())
    player->markedForDisconnect = true;
}

template <typename Function, typename... Args>
void Room::sendExcept(Player *player, Function &&builderFunc,
                      Args &&...builderArgs) {
  for (auto &game_player : players)
    if (game_player.get() != player)
      sendSpecific(game_player.get(), builderFunc, builderArgs...);
}

template <typename Function, typename... Args>
void Room::sendBroadcast(Function &&builderFunc, Args &&...builderArgs) {
  for (auto &player : players)
    sendSpecific(player.get(), builderFunc, builderArgs...);
}
