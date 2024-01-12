#ifndef BOOMBERMAN_BOOMBERMAN_H
#define BOOMBERMAN_BOOMBERMAN_H

#include "../shared/Util.h"
#include "../shared/game/Map.h"
#include <memory>
#include <raylib.h>
#include <string>

class Boomberman {
private:
  int *position;
  int state;
  double long animation_start{};

public:
  int iframes, health;
  Color color{};
  std::string
      pseudonim_artystyczny_według_którego_będzie_się_identyfikował_wśród_społeczności_graczy;
  Boomberman(
      const std::string &
          pseudonim_artystyczny_według_którego_będzie_się_identyfikował_wśród_społeczności_graczy,
      Color color, int start_x, int start_y, int health);
  std::unique_ptr<int[]> getBoombermanPos();
  void setBoombermanPos(int new_x, int new_y);
  bool move(Map *map, std::shared_ptr<float[]> curr_pos, int x, int y);
  void cleanUp();
  void gotHit(int64_t when);
  void decrementIframes();
  void animateHit();
  [[nodiscard]] int getState() const;
};

#endif // BOOMBERMAN_BOOMBERMAN_H
