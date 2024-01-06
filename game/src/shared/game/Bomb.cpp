#include "Bomb.h"
#include <chrono>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <stdexcept>
#include <sys/timerfd.h>

Bomb::Bomb(int pos_x, int pos_y, int explosion, int size, long double timestamp,
           float ttl, bool is_atomic) {
  this->pos_x = pos_x;
  this->pos_y = pos_y;
  this->explosion = explosion;
  this->size = size;
  this->ttl = ttl;
  this->animation_start = timestamp;
  this->plant_time = this->animation_start;
  this->state = -1;
  this->is_atomic = is_atomic;
}

bool Bomb::ShouldExplode() const {
  return Util::TimestampMillis() > this->plant_time + this->ttl * 1000;
}

void Bomb::animate() {
  double long now = Util::TimestampMillis();
  if (now - this->animation_start >= 500.0f) {
    this->state *= -1;
    this->animation_start = now;
  }
}
std::vector<TileOnFire> Bomb::boom(Map *map) {
  std::vector<TileOnFire> tilesLitUp;
  tilesLitUp.emplace_back(this->pos_x, this->pos_y);
  for (int direction = -1; direction <= 1; direction += 2) {
    for (int x = 1; x < this->explosion; x++) {
      int may_be_wall =
          map->getSquareState(this->pos_x + x * direction, this->pos_y);
      if (may_be_wall == 1)
        break;
      else if (may_be_wall == 2) {
        map->setSquareState(this->pos_x + x * direction, this->pos_y, 0);
        tilesLitUp.emplace_back(this->pos_x + x * direction, this->pos_y);
        if (!this->is_atomic)
          break;
      } else
        tilesLitUp.emplace_back(this->pos_x + x * direction, this->pos_y);
    }
    for (int y = 1; y < this->explosion; y++) {
      int may_be_wall =
          map->getSquareState(this->pos_x, this->pos_y + y * direction);
      if (may_be_wall == 1)
        break;
      else if (may_be_wall == 2) {
        map->setSquareState(this->pos_x, this->pos_y + y * direction, 0);
        tilesLitUp.emplace_back(this->pos_x, this->pos_y + y * direction);
        if (!this->is_atomic)
          break;
      } else
        tilesLitUp.emplace_back(this->pos_x, this->pos_y + y * direction);
    }
  }
  return tilesLitUp;
}

int Bomb::CreateBombTimerfd() {
  int timerfd = timerfd_create(CLOCK_REALTIME, 0);
  if (timerfd == -1)
    throw std::runtime_error("timerfd create");

  struct itimerspec new_value;
  memset(&new_value, 0, sizeof(new_value));
  new_value.it_value.tv_sec = FUSE_TIME_SEC;

  if (timerfd_settime(timerfd, 0, &new_value, nullptr) == -1)
    throw std::runtime_error("timerfd set");
  return timerfd;
}
