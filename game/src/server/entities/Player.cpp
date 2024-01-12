#include "Player.h"
#include "../../shared/Util.h"
#include <cmath>
#include <optional>

Player::Player(Connection *conn, std::string username, PlayerColor color) {
  this->conn = conn;
  this->username = std::move(username);
  this->color = color;
  this->livesRemaining = STARTER_LIVES;
  this->immunityEndTimestamp = Util::TimestampMillis();

  switch (color) {
  case PLAYER_RED:
    this->coords = Coords{1.0f, 1.0f};
    break;
  case PLAYER_GREEN:
    this->coords = Coords{15.0f, 1.0f};
    break;
  case PLAYER_BLUE:
    this->coords = Coords{1.0f, 9.0f};
    break;
  case PLAYER_YELLOW:
    this->coords = Coords{15.0f, 9.0f};
    break;
  default:
    throw std::runtime_error("unhandled color starting position");
  }
}

Coords Player::GetCoords() { return coords; }

std::optional<Coords> Player::MoveCheckSus(int x, int y) {
  int move_x = std::abs(x - coords.x);
  int move_y = std::abs(y - coords.y);
  if (move_x && move_y)
    return coords; // You can only move diagonal

  step++;
  if (step < 6) {
    coords.x = x;
    coords.y = y;
    return std::nullopt; // No need to correct
  }

  // check if the movement 4 moves ago
  Timestamp ts = Util::TimestampMillis();
  int oldest_step = (step % 5) == 4 ? 0 : step + 1;
  int diff = ts - moveHistory[oldest_step].ts < PLAYER_SUS_TRESHHOLD_MILLIS;
  if (diff > 0 && diff < PLAYER_SUS_TRESHHOLD_MILLIS) {
    step = 0; // reset steps
    return moveHistory[oldest_step].coords;
  }

  coords.x = x;
  coords.y = y;
  moveHistory[step % 5] = MovementEntry{ts, coords};
  return std::nullopt; // No need to correct
}
