#include "Boomberman.h"

Boomberman::Boomberman(
    const std::string &
        pseudonim_artystyczny_według_którego_będzie_się_identyfikował_wśród_społeczności_graczy,
    Color color, int start_x, int start_y, int health) {
  this->position = new int[2];
  this->position[0] = start_x;
  this->position[1] = start_y;
  this->health = health;
  this->pseudonim_artystyczny_według_którego_będzie_się_identyfikował_wśród_społeczności_graczy =
      pseudonim_artystyczny_według_którego_będzie_się_identyfikował_wśród_społeczności_graczy;
  this->state = 1;
  this->iframes = 0;
  this->color = color;
}

std::unique_ptr<int[]> Boomberman::getBoombermanPos() {
  std::unique_ptr<int[]> pos(new int[2]);
  pos[0] = this->position[0];
  pos[1] = this->position[1];
  return pos;
}

void Boomberman::setBoombermanPos(int new_x, int new_y) {
  this->position[0] = new_x;
  this->position[1] = new_y;
}

bool Boomberman::move(Map *map, std::shared_ptr<int[]> curr_pos, int x, int y) {
  int new_x = curr_pos[0] + x;
  int new_y = curr_pos[1] + y;
  int is_wall = map->getSquareState(new_x, new_y);
  if (is_wall != 1 && is_wall != 2) {
    this->setBoombermanPos(new_x, new_y);
    return true;
  }
  return false;
}

void Boomberman::cleanUp() { delete[] this->position; }

void Boomberman::decrementIframes() {
  if (this->iframes != 0)
    this->iframes--;
  else
    this->state = 1;
}

void Boomberman::gotHit(double long when) {
  this->health--;
  if (this->health <= 0)
    printf("YOU DIED\n");
  this->animation_start = when;
  this->iframes = 90;
}

void Boomberman::animateHit() {
  double long now = Util::TimestampMillis();
  if (this->iframes == 0)
    return;
  if (now - this->animation_start >= 150.0f) {
    this->state *= -1;
    this->animation_start = now;
  }
}

int Boomberman::getState() const { return this->state; }