#include "Util.h"
#include <chrono>
#include <random>

Timestamp util::TimestampMillis() {
  return static_cast<Timestamp>(
      std::chrono::duration_cast<std::chrono::milliseconds>(
          std::chrono::system_clock::now().time_since_epoch())
          .count());
}

// Thanks
// https://stackoverflow.com/questions/440133/how-do-i-create-a-random-alpha-numeric-string-in-c
std::string util::RandomString(std::string::size_type length) {
  static auto &chrs =
      "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
  thread_local static std::mt19937 rg{std::random_device{}()};
  thread_local static std::uniform_int_distribution<std::string::size_type>
      pick(0, sizeof(chrs) - 2);

  std::string result;
  result.reserve(length);
  while (length--)
    result += chrs[pick(rg)];
  return result;
}
