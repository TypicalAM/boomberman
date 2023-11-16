#include "SBomb.h"
#include "../../shared/Util.h"

bool SBomb::ShouldExplode() const {
    return Util::TimestampMillis() > fuseStartTimestamp + FUSE_TIME_MILLIS;
}

bool SBomb::InBlastRadius(const SPlayer &player) const {
    Coords pcoords = player.coords;
    return ((this->coords.x - 3) <= pcoords.x && pcoords.x <= (this->coords.x + 3)) &&
           ((this->coords.y - 3) <= pcoords.y && pcoords.y <= (this->coords.y + 3));
}

SBomb::SBomb(Coords coords, int64_t fuse) {
    this->coords = coords;
    this->fuseStartTimestamp = fuse;
}