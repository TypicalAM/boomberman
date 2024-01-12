#ifndef BOOMBERMAN_CLIENT_H
#define BOOMBERMAN_CLIENT_H

#include "../shared/game/Map.h"
#include <string>

class Client {
private:
  float width;
  float height;

public:
  Client(float width, float height);
  static void drawMap(Map *map);
  void Run(const std::string &, int port) const;

  ~Client();
};
#endif
