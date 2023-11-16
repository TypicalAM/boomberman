#ifndef BOOMBERMAN_UTIL_H
#define BOOMBERMAN_UTIL_H

#include <cstdint>
#include <string>

namespace Util {
    int64_t TimestampMillis();
    std::string RandomString(std::string::size_type length);
};

#endif