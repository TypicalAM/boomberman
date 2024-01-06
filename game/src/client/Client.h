#ifndef BOOMBERMAN_CLIENT_H
#define BOOMBERMAN_CLIENT_H

#include "../shared/game/Map.h"
#include "ServerHandler.h"
#include <string>

class Client {
private:
  int width;
  int height;

public:
  Client(int width, int height);
  static void drawMap(Map *map);
  void Run() const;
  int getDimension(const std::string &dimension) const;

  ~Client();
};
#endif