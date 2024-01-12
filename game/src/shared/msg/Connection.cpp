#include "Connection.h"
#include <asm-generic/socket.h>
#include <cstddef>
#include <cstdint>
#include <error.h>
#include <memory>
#include <optional>
#include <stdexcept>
#include <sys/socket.h>
#include <unistd.h>

Connection::Connection(int sock) {
  struct timeval timeout;
  timeout.tv_sec = CONN_TIMEOUT_MILLIS % 1000;
  timeout.tv_usec = 1000 * (CONN_TIMEOUT_MILLIS % 1000);

  if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout) < 0)
    throw std::runtime_error("set read timeout on sock");

  if (setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof timeout) < 0)
    throw std::runtime_error("set write timeout on sock");

  this->sock = sock;
  this->msg = std::make_unique<GameMessage>();
}

std::optional<std::unique_ptr<GameMessage>> Connection::Receive() {
  // The queue first
  if (!inboundQueue.empty()) {
    auto new_msg = std::move(inboundQueue.front());
    inboundQueue.pop();
    return new_msg;
  }

  char buf[256]{};
  size_t bytes_received = read(sock, buf, 256);
  if (bytes_received <= 0)
    return std::nullopt;

  // Now we have three possible cases
  // We can receive 1 full message (god bless)
  // We can receive n full messages, in this case we fill the inboundQueue with
  // messages and expect the client of the function to check if there are more
  // We can receive n full and a fraction of the next message (worst case), then
  // we wait for the other fraction of the message to appear?

  auto msg_size = static_cast<uint8_t>(buf[0]);
  if (bytes_received == msg_size + 1) {
    // Everything is fine, let's deserialize and return
    GameMessage new_msg;
    new_msg.ParseFromArray(buf + 1, msg_size);
    return std::make_unique<GameMessage>(new_msg);
  }

  while (bytes_received >= msg_size + 1) {
    // We received more than one message (maybe one is partial)
    GameMessage new_msg;
    new_msg.ParseFromArray(buf + 1, msg_size);
    inboundQueue.push(std::make_unique<GameMessage>(new_msg));

    // Now we decrement the bytes received for the second message and
    // recalculate the size
    bytes_received -= msg_size + 1;
    if (bytes_received == 0)
      break;
    memmove(buf, buf + msg_size + 1, 256 - msg_size - 1);
    msg_size = static_cast<uint8_t>(buf[0]);
  }

  if (bytes_received == 0) {
    // We've read n full messages
    auto result = std::move(inboundQueue.front());
    inboundQueue.pop();
    return result;
  }

  // NOTE: I know this isn't the best solution, because there is the edge
  // case of a timeout in the middle of receiving a stream. This could
  // potentially lag the read thread of the server until the client breaks the
  // connection or reconnects. NOTE: Connection doesn't intend to support
  // non-blocking reads.

  std::cerr << "[Connection] Couldn't read enough data, read " << bytes_received
            << "/" << msg_size + 1 << std::endl;
  std::cerr << "[Connection] Continuing to read" << std::endl;

  // Read until we get the message
  uint8_t read_total = bytes_received;
  while (read_total < msg_size + 1) {
    bytes_received = read(sock, buf + read_total, 1);
    if (bytes_received <= 0)
      return std::nullopt;
    read_total += bytes_received;
  }

  GameMessage new_msg;
  new_msg.ParseFromArray(buf + 1, msg_size);
  inboundQueue.push(std::make_unique<GameMessage>(new_msg));

  auto result = std::move(inboundQueue.front());
  inboundQueue.pop();
  return result;
}

bool Connection::HasMoreMessages() { return !inboundQueue.empty(); }

std::optional<int> Connection::SendError(std::string error) {
  auto e = std::make_unique<Error>();
  e->set_error(error);
  msg->set_type(ERROR);
  msg->set_allocated_error(e.release());
  return send();
}

std::optional<int> Connection::SendGetRoomList() {
  auto grl = std::make_unique<GetRoomList>();
  msg->set_type(GET_ROOM_LIST);
  msg->set_allocated_getroomlist(grl.release());
  return send();
}

std::optional<int>
Connection::SendJoinRoom(const std::string &name,
                         std::optional<std::string> roomName) {
  auto jr = std::make_unique<JoinRoom>();
  jr->set_username(name);
  if (roomName.has_value())
    jr->set_roomname(roomName.value());
  msg->set_type(JOIN_ROOM);
  msg->set_allocated_joinroom(jr.release());
  return send();
}

std::optional<int> Connection::SendIPlaceBomb(float x, float y) {
  auto ipb = std::make_unique<IPlaceBomb>();
  ipb->set_x(x);
  ipb->set_y(y);

  msg->set_type(I_PLACE_BOMB);
  msg->set_allocated_iplacebomb(ipb.release());
  return send();
}

std::optional<int> Connection::SendIMove(float x, float y) {
  auto im = std::make_unique<IMove>();
  im->set_x(x);
  im->set_y(y);

  msg->set_type(I_MOVE);
  msg->set_allocated_imove(im.release());
  return send();
}

std::optional<int> Connection::SendILeave() {
  msg->set_type(I_LEAVE);
  return send();
}

std::optional<int>
Connection::SendRoomList(const std::vector<builder::Room> &rooms) {
  auto rl = std::make_unique<RoomList>();
  for (const auto &room : rooms) {
    GameRoom *r = rl->add_rooms();
    r->set_name(room.name);
    r->set_playercount(room.players);
    r->set_maxplayercount(room.maxPlayers);
  }

  msg->set_type(ROOM_LIST);
  msg->set_allocated_roomlist(rl.release());
  return send();
}

std::optional<int>
Connection::SendWelcomeToRoom(const std::vector<builder::Player> &players) {
  auto wtr = std::make_unique<WelcomeToRoom>();
  for (const auto &player : players) {
    GamePlayer *gp = wtr->add_players();
    gp->set_username(player.username);
    gp->set_color(player.color);
  }

  msg->set_type(WELCOME_TO_ROOM);
  msg->set_allocated_welcometoroom(wtr.release());
  return send();
}

std::optional<int> Connection::SendGameJoin(builder::Player player) {
  auto p = std::make_unique<GamePlayer>();
  p->set_username(player.username);
  p->set_color(player.color);

  auto gj = std::make_unique<GameJoin>();
  gj->set_allocated_player(p.release());

  msg->set_type(GAME_JOIN);
  msg->set_allocated_gamejoin(gj.release());
  return send();
}

std::optional<int> Connection::SendGameStart() {
  msg->set_type(GAME_START);
  return send();
}

std::optional<int> Connection::SendOtherBombPlace(std::string username,
                                                  int64_t timestamp, float x,
                                                  float y) {
  auto obp = std::make_unique<OtherBombPlace>();
  obp->set_username(username);
  obp->set_timestamp(timestamp);
  obp->set_x(x);
  obp->set_y(y);

  msg->set_type(OTHER_BOMB_PLACE);
  msg->set_allocated_otherbombplace(obp.release());
  return send();
}

std::optional<int> Connection::SendGotHit(const std::string &username,
                                          int32_t livesRemaining,
                                          int64_t timestamp) {
  auto gh = std::make_unique<GotHit>();
  gh->set_username(username);
  gh->set_livesremaining(livesRemaining);
  gh->set_timestamp(timestamp);

  msg->set_type(GOT_HIT);
  msg->set_allocated_gothit(gh.release());
  return send();
}

std::optional<int> Connection::SendOtherMove(std::string name, float x,
                                             float y) {
  auto om = std::make_unique<OtherMove>();
  om->set_username(name);
  om->set_x(x);
  om->set_y(y);

  msg->set_type(OTHER_MOVE);
  msg->set_allocated_othermove(om.release());
  return send();
}

std::optional<int> Connection::SendOtherLeave(const std::string &username) {
  auto ol = std::make_unique<OtherLeave>();
  ol->set_username(username);

  msg->set_type(OTHER_LEAVE);
  msg->set_allocated_otherleave(ol.release());
  return send();
}

std::optional<int> Connection::SendGameWon(std::string winnerUsername) {
  auto gw = std::make_unique<GameWon>();
  gw->set_winnerusername(winnerUsername);

  msg->set_type(GAME_WON);
  msg->set_allocated_gamewon(gw.release());
  return send();
}

std::optional<int> Connection::SendMovementCorrection(float x, float y) {
  auto mc = std::make_unique<MovementCorrection>();
  mc->set_x(x);
  mc->set_y(y);

  msg->set_type(MOVEMENT_CORRECTION);
  msg->set_allocated_movementcorrection(mc.release());
  return send();
}

std::optional<int> Connection::send() {
  char buf[256]{};
  size_t msg_size = msg->ByteSizeLong();
  if (msg_size > 255)
    throw std::runtime_error(
        "incorrect message size"); // We handle this just in case, we don't send
                                   // that big of a message
  buf[0] = static_cast<uint8_t>(msg_size);
  msg->SerializeToArray(buf + 1, msg_size);
  size_t bytes_sent = write(sock, buf, msg_size + 1);
  if (bytes_sent <= 0 || bytes_sent > 255)
    return std::nullopt;

  if (bytes_sent == msg_size + 1) // best situation
    return bytes_sent;

  std::cerr << "[Connection] Couldn't send enough data, sent " << bytes_sent
            << "/" << msg_size + 1 << std::endl;
  std::cerr << "[Connection] Continuing to send" << std::endl;

  size_t total_sent = bytes_sent;
  while (total_sent != msg_size + 1) {
    size_t bytes_sent =
        write(sock, buf + total_sent, msg_size + 1 - total_sent);
    if (bytes_sent <= 0)
      return std::nullopt;

    total_sent += bytes_sent;
  }

  return total_sent;
}
