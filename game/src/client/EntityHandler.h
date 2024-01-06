#ifndef BOOMBERMAN_ENTITYHANDLER_H
#define BOOMBERMAN_ENTITYHANDLER_H

#include "../shared/game/Bomb.h"
#include "Boomberman.h"

class EntityHandler {
public:
  std::vector<Boomberman> players;
  std::vector<Bomb> bombs;
  std::vector<TileOnFire> theFloorIsLava;

  void placeBomb(int x, int y, int explosion, int size, long double timestamp,
                 int ttl, bool is_atomic);
  // static std::vector<TileOnFire> setOnFire(Bomb* bomb, Map* map);
  int explodeBomb(Bomb *bomb, Map *map);
  void tryExtinguish();

  void drawPlayers(Map *map);
  void drawBombs(Map *map);
  void drawFire(Map *map);
};

#endif
