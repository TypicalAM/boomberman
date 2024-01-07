#ifndef BOOMBERMAN_SERVER_H
#define BOOMBERMAN_SERVER_H

#include "Room.h"
#include <mutex>
#include <optional>
#include <unordered_map>

#define MAX_EVENTS 25

struct PlayerInRoom {
  SPlayer *player;
  Room *room;
};

class Server {
private:
  std::unordered_map<std::string, std::shared_ptr<Room>> rooms;
  std::unordered_map<int, std::unique_ptr<Connection>> roomConns;
  std::unordered_map<int, PlayerInRoom> roomAssignments;
  int roomEpollSock;
  int bombEpollSock;

  std::vector<std::unique_ptr<Connection>> lobbyConns;
  std::vector<int> lobbySockets;
  std::mutex roomsMtx;
  std::mutex runMtx;
  int srvSock;
  int lobbyEpollSock;
  boost::log::sources::logger logger;

  std::atomic<bool> end = false;

  void handleClientMessage(Connection *conn, std::unique_ptr<GameMessage> msg);

public:
  static boost::log::sources::logger createNamedLogger(const std::string &name);
  void RunLobby();
  void RunRooms();
  void RunBombs();
  void Run();
  void Shutdown();

  explicit Server(int port);
};

#endif
