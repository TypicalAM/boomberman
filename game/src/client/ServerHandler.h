#ifndef BOOMBERMAN_SERVERHANDLER_H
#define BOOMBERMAN_SERVERHANDLER_H

#include <arpa/inet.h>
#include <error.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/poll.h>
#include <sys/socket.h>

#include "../shared/msg/Connection.h"
#include "EntityHandler.h"
#include "raylib.h"

class ServerHandler {
private:
  int start_x{}, start_y{};
  pollfd polling[1]{};
  Color start_color{};

public:
  std::unique_ptr<Connection> conn;
  ServerHandler();
  void connect2Server(const char *ip, int port) const;
  static std::string selectUsername(float width, float height);
  void menu(float width, float height);
  void setPlayerParams(const GamePlayer &player);
  void wait4Game(EntityHandler &eh, float width, float height);
  void addPlayer(const GamePlayer &player, EntityHandler &eh);
  void joinRoom(EntityHandler &eh, std::unique_ptr<GameMessage> msg);
  void listRooms(float width, float height, std::unique_ptr<GameMessage> msg);
  bool handleLobbyMsg(EntityHandler &eh, float width, float height,
                      std::unique_ptr<GameMessage> msg);

  [[noreturn]] void receiveLoop(EntityHandler &eh);
  static std::vector<Boomberman>::iterator
  findPlayer(EntityHandler &eh, const std::string &username);
  void handleMessage(EntityHandler &eh, std::unique_ptr<GameMessage> msg);
};

#endif // BOOMBERMAN_SERVERHANDLER_H
