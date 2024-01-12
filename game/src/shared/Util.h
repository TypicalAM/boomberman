#ifndef BOOMBERMAN_UTIL_H
#define BOOMBERMAN_UTIL_H

#include <cstdint>
#include <string>

typedef int64_t Timestamp;

/**
 * \namespace util
 * \brief The random utilities namespace
 */
namespace util {
/*
 * Get the current milliseconds unix timestamp
 * @return timestamp
 */
Timestamp TimestampMillis();

/*
 * Generate a variadic length random string
 * @return random string
 */
std::string RandomString(std::string::size_type length);
}; // namespace util

#endif
