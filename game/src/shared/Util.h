#ifndef BOOMBERMAN_UTIL_H
#define BOOMBERMAN_UTIL_H

#include <cstdint>
#include <string>

typedef int64_t Timestamp;

namespace Util {
Timestamp TimestampMillis();

std::string RandomString(std::string::size_type length);
}; // namespace Util

#endif