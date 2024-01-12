#ifndef BOOMBERMAN_SERVER_H
#define BOOMBERMAN_SERVER_H

#include "Room.h"
#include <mutex>
#include <sys/epoll.h>
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
  std::mutex roomsMtx;
  int srvSock;
  int lobbyEpollSock;
  boost::log::sources::logger logger;

  int sigSock; // NOTE: we use that to catch a stopping signal because closing
               // an epoll doesnt end its epoll_waits (xd)
  std::atomic<bool> end = false;

  void handleLobbyMessage(Connection *conn, std::unique_ptr<GameMessage> msg);

  bool shouldEnd(epoll_event &event);

  void handleRoomPostEvent(Room *room);

  void cleanupSock(int sock);

public:
  static boost::log::sources::logger createNamedLogger(const std::string &name);
  void RunLobby();
  void RunRooms();
  void RunBombs();
  void Run();
  void Cleanup();

  explicit Server(int port);
};

#endif
