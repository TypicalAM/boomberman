#ifndef BOOMBERMAN_SERVER_H
#define BOOMBERMAN_SERVER_H

#include "Room.h"
#include <mutex>
#include <sys/epoll.h>
#include <unordered_map>

#define MAX_EVENTS 25

/**
 * @struct PlayerInRoom
 * Allows us to track the player's rooms without searching maps/vectors all the
 * time
 */
struct PlayerInRoom {
  Player *player;
  Room *room;
};

/**
 * @struct BombInRoom
 * Allows us to track the bombs's rooms without searching maps/vectors all the
 * time
 */
struct BombInRoom {
  int fd;
  Room *room;
};

/*
 * @class Server
 * Main hub for managing the state of rooms, bombs and the lobby
 */
class Server {
public:
  /*
   * A constructor for the Server
   * @param port TCP port to run on
   */
  explicit Server(int port);

  /*
   * Run the server, wait until all the run threads are finished
   */
  void Run();

  /*
   * Clean up after the server is finsihed
   */
  void Cleanup();

  /*
   * Create a named logger, so log messages will be easily readable
   * something like [ROOMNAME] Hello!
   * @return a named logger
   */
  static boost::log::sources::logger CreateNamedLogger(const std::string &name);

private:
  boost::log::sources::logger logger;
  std::atomic<bool> end = false;
  std::unordered_map<std::string, std::shared_ptr<Room>> rooms;
  std::unordered_map<int, std::unique_ptr<Connection>> roomConns;
  std::unordered_map<int, PlayerInRoom> roomAssignments;
  std::vector<std::unique_ptr<Connection>> lobbyConns;
  std::mutex roomsMtx;
  int roomEpollSock;
  int lobbyEpollSock;
  int bombEpollSock;
  int srvSock;
  int sigSock; // NOTE: we use that to catch a stopping signal because closing
               // an epoll doesnt end its epoll_waits (xd)

  void runLobby();
  void runRooms();
  void runBombs();
  void handleLobbyMessage(Connection *conn, std::unique_ptr<GameMessage> msg);
  bool shouldEnd(epoll_event &event);
  void handleRoomPostEvent(Room *room);
  void cleanupSock(int sock);
};

#endif
