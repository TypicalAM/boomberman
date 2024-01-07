#include "Server.h"
#include <boost/log/attributes/constant.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <csignal>
#include <netinet/in.h>
#include <ratio>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

void Server::handleClientMessage(Connection *conn,
                                 std::unique_ptr<GameMessage> msg) {
  std::lock_guard<std::mutex> lock(roomsMtx);
  switch (msg->type()) {
  case GET_ROOM_LIST: {
    // Create a room list and send it to the user
    std::vector<Builder::Room> room_list;
    std::vector<std::string> rooms_finished;
    for (const auto &pair : rooms) {
      if (pair.second->IsGameOver())
        rooms_finished.push_back(pair.first);
      int player_count = pair.second->PlayersCount();
      if (player_count != MAX_PLAYERS)
        room_list.push_back(
            Builder::Room{pair.first, player_count, MAX_PLAYERS});
    }

    for (const auto &key : rooms_finished) {
      LOG << "Destroyed a finished room: " << key;
      rooms.erase(key);
    }

    // Close the connection if we can't send the message
    if (!conn->SendRoomList(room_list).has_value()) {
      epoll_ctl(lobbyEpollSock, EPOLL_CTL_DEL, conn->sock, nullptr);
      shutdown(conn->sock, SHUT_RDWR);
      close(conn->sock);
    }
    return;
  }

  case JOIN_ROOM: {
    // Join a user to a room (if he specified the room) or create a new one
    auto room_msg = msg->joinroom();

    std::shared_ptr<Room> room;
    if (!room_msg.has_roomname()) {
      auto new_room_name = Util::RandomString(10);
      room = std::make_shared<Room>(createNamedLogger(new_room_name));
      rooms[new_room_name] = room;
    } else {
      if (rooms.find(room_msg.roomname()) == rooms.end()) {
        // Close the connection if we can't send the message
        LOG << "Client requested a room which doesn't exist: "
            << room_msg.roomname();
        if (conn->SendError("There is no room with that name man").has_value())
          return;
        epoll_ctl(lobbyEpollSock, EPOLL_CTL_DEL, conn->sock, nullptr);
        shutdown(conn->sock, SHUT_RDWR);
        close(conn->sock);
        return;
      }

      room = rooms[room_msg.roomname()];
    }

    if (!room->CanJoin(room_msg.username())) {
      // Close the connection if we can't send the message
      if (conn->SendError("Cannot join this game").has_value())
        return;
      epoll_ctl(lobbyEpollSock, EPOLL_CTL_DEL, conn->sock, nullptr);
      shutdown(conn->sock, SHUT_RDWR);
      close(conn->sock);
      return;
    }

    LOG << "Joining player to game: " << room_msg.username();

    // Delete the old epoll
    epoll_ctl(lobbyEpollSock, EPOLL_CTL_DEL, conn->sock, nullptr);

    // Find the index and move the connection from the lobby into the room
    int idx = -1;
    for (int i = 0; i < lobbyConns.size(); i++)
      if (lobbyConns[i]->sock == conn->sock)
        idx = i;
    roomConns[conn->sock] = std::move(lobbyConns[idx]);
    lobbyConns.erase(lobbyConns.begin() + idx);

    // Create the new player
    SPlayer *player = room->JoinPlayer(
        conn, room_msg.username()); // player is destructed along with the room
    roomAssignments[conn->sock] = PlayerInRoom{player, room.get()};
    epoll_event event = {EPOLLIN | EPOLLET,
                         epoll_data{.ptr = &roomAssignments[conn->sock]}};
    epoll_ctl(roomEpollSock, EPOLL_CTL_ADD, conn->sock,
              &event); // TODO: Error handling
    return;
  }

  default: {
    // Close the connection if we can't send the message
    LOG << "Unexpected message type: " << msg->type();
    if (!conn->SendError("Unexpected message").has_value()) {
      epoll_ctl(lobbyEpollSock, EPOLL_CTL_DEL, conn->sock, nullptr);
      shutdown(conn->sock, SHUT_RDWR);
      close(conn->sock);
    }
  }
  }
}

void Server::RunBombs() {
  LOG << "Serving bombs";

  epoll_event events[MAX_EVENTS];
  while (!end.load()) {
    int event_num = epoll_wait(bombEpollSock, events, MAX_EVENTS, -1);
    for (int i = 0; i < event_num; ++i) {
      auto room = static_cast<Room *>(events[i].data.ptr);
      LOG << "Got new bomb timer expiry, notifying appropriate room";
      std::lock_guard<std::mutex> lock(roomsMtx); // ptr read isn't atomic
      room->ExplodeBomb();
    }
  }
}

void Server::RunRooms() {
  LOG << "Serving rooms";

  epoll_event events[MAX_EVENTS];
  while (!end.load()) {
    int event_num = epoll_wait(roomEpollSock, events, MAX_EVENTS, -1);
    for (int i = 0; i < event_num; ++i) {
      std::lock_guard<std::mutex> lock(roomsMtx);
      auto pl = static_cast<PlayerInRoom *>(events[i].data.ptr);
      LOG << "Got new message from guy: " << pl->player->username;

      auto msg = pl->player->conn->Receive();
      if (!msg.has_value()) {
        // TODO: Close the connection and send I_LEAVE, place super bomb, etc.
        pl->room->PlaceSuperBomb(pl->player);
        LOG << "Received nothin";
        shutdown(pl->player->conn->sock, SHUT_RDWR);
        close(pl->player->conn->sock);
        continue;
      }

      LOG << "Received a message of type: " << msg.value()->type();
      auto authored = std::make_unique<AuthoredMessage>(
          AuthoredMessage{std::move(msg.value()), pl->player});
      bool place_bomb = pl->room->HandleMessage(std::move(authored));
      if (!place_bomb)
        continue;

      // Create a timer based on the timestamp and set a watch for it
      epoll_event event = {EPOLLIN | EPOLLET, epoll_data{.ptr = pl->room}};
      epoll_ctl(bombEpollSock, EPOLL_CTL_ADD, Bomb::CreateBombTimerfd(),
                &event); // TODO: Error handling
    }
  }
}

void Server::RunLobby() {
  LOG << "Serving lobby";

  epoll_event events[MAX_EVENTS];
  while (!end.load()) {
    int event_num = epoll_wait(lobbyEpollSock, events, MAX_EVENTS, -1);
    for (int i = 0; i < event_num; ++i) {
      // If this isn't an in event, ignore it
      if (!(events[i].events & EPOLLIN))
        continue;

      // Check if server event
      if (events[i].data.fd == srvSock) {
        int new_sock = accept(srvSock, nullptr, nullptr);
        if (new_sock == -1)
          continue;
        lobbyConns.push_back(std::make_unique<Connection>(new_sock));
        epoll_event event = {EPOLLIN | EPOLLET,
                             epoll_data{.ptr = lobbyConns.back().get()}};
        if (epoll_ctl(lobbyEpollSock, EPOLL_CTL_ADD, new_sock, &event) == -1)
          continue;
        LOG << "New connection accepted from: " << new_sock;
        continue;
      }

      // We got a message from a client
      auto conn = static_cast<Connection *>(events[i].data.ptr);
      auto msg = conn->Receive();
      if (!msg.has_value()) {
        LOG << "Closing connection since we can't receive data: " << conn->sock;
        epoll_ctl(lobbyEpollSock, EPOLL_CTL_DEL, events[i].data.fd, nullptr);
        shutdown(events[i].data.fd, SHUT_RDWR);
        close(events[i].data.fd);
        continue;
      }

      // Handle the message
      handleClientMessage(conn, std::move(msg.value()));
    }
  }
}

boost::log::sources::logger Server::createNamedLogger(const std::string &name) {
  boost::log::sources::logger room_logger;
  room_logger.add_attribute(
      "Prefix",
      boost::log::attributes::constant<std::string>("[" + name + "] "));
  return room_logger;
}

Server::Server(int port) {
  logger = createNamedLogger("Server");
  sockaddr_in localAddress{AF_INET, htons(port), htonl(INADDR_ANY)};
  int one = 1;
  // Socket creation
  if ((srvSock = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
    throw std::runtime_error("socket creation failed");
  }

  // Set socket options
  if (setsockopt(srvSock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)) == -1) {
    close(srvSock); // Clean up on failure
    throw std::runtime_error("setting socket options failed");
  }

  // Bind the socket
  if (bind(srvSock, (sockaddr *)&localAddress, sizeof(localAddress)) == -1) {
    close(srvSock); // Clean up on failure
    throw std::runtime_error("bind failed");
  }

  // Listen for incoming connections
  if (listen(srvSock, 1) == -1) {
    close(srvSock);
    throw std::runtime_error("listen failed");
  }

  // Create lobby epoll instance
  if ((lobbyEpollSock = epoll_create1(0)) == -1) {
    close(lobbyEpollSock); // Clean up on failure
    throw std::runtime_error("epoll creation failed");
  }

  // Add server socket to lobby epoll
  epoll_event event = {EPOLLIN | EPOLLET, epoll_data{.fd = srvSock}};
  if (epoll_ctl(lobbyEpollSock, EPOLL_CTL_ADD, srvSock, &event) == -1) {
    close(srvSock);        // Clean up on failure
    close(lobbyEpollSock); // Clean up on failure
    throw std::runtime_error("epoll control failed");
  }

  // Create room epoll instance
  if ((roomEpollSock = epoll_create1(0)) == -1) {
    close(roomEpollSock); // Clean up on failure
    close(lobbyEpollSock);
    throw std::runtime_error("epoll creation failed");
  }

  // Create bombs epoll instance
  if ((bombEpollSock = epoll_create1(0)) == -1) {
    close(bombEpollSock); // Clean up on failure
    close(roomEpollSock);
    close(lobbyEpollSock);
    throw std::runtime_error("epoll creation failed");
  }

  LOG << "Listening on port: " << port;
}

void Server::Shutdown() {
  end.store(true);
  std::lock_guard<std::mutex> lock(runMtx); // wait until Run() ends

  for (const auto &[name, room] : rooms) {
    for (const auto &player : room->players) {
      shutdown(player->conn->sock, SHUT_RDWR);
      close(player->conn->sock);
    }
  }

  close(bombEpollSock);
  close(lobbyEpollSock);
  close(roomEpollSock);
  close(srvSock);
}

void Server::Run() {
  std::lock_guard<std::mutex> lock(runMtx);
  std::thread rooms(&Server::RunRooms, this);
  std::thread bombs(&Server::RunBombs, this);
  std::thread lobby(&Server::RunLobby, this);

  rooms.join();
  bombs.join();
  lobby.join();
}
