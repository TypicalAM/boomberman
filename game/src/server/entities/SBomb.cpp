#include "SBomb.h"

bool SBomb::ShouldExplode() const {
    int64_t currentMillis = static_cast<int64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count());
    return fuseStartTimestamp + FUSE_TIME_MILLIS < currentMillis;
}

bool SBomb::InBlastRadius(const SPlayer& player) const {
    Coords pcoords = player.coords;
    return ((this->coords.x - 3) <= pcoords.x && pcoords.x <= (this->coords.x + 3)) &&
           ((this->coords.y - 3) <= pcoords.y && pcoords.y <= (this->coords.y + 3));
}

SBomb::SBomb(Coords coords, int64_t fuse) {
    this->coords = coords;
    this->fuseStartTimestamp = fuse;
}