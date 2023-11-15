#ifndef BOOMBERMAN_SBOMB_H
#define BOOMBERMAN_SBOMB_H

#include <cstdint>
#include <vector>
#include "Primitives.h"
#include "SPlayer.h"

#define FUSE_TIME_MILLIS 3000

class SBomb {
public:
    Coords coords{};
    int64_t fuseStartTimestamp;

    bool ShouldExplode() const;

    bool InBlastRadius(const SPlayer& player) const;

    SBomb(Coords coords, int64_t fuse);
};

#endif