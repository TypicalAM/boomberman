#include "YouGotHit.h"

std::string YouGotHit::name() const {
    return "You got hit";
}

MessageType YouGotHit::type() const {
    return YOUGOTHIT;
}
