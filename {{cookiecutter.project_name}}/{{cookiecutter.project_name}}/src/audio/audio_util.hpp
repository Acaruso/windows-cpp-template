#pragma once

#include <string>
#include <sstream>

#include "audio_constants.hpp"

template <typename T>
inline std::string to_hex(const T& t) {
    std::stringstream ss;
    ss << "0x" << std::hex << t;
    return ss.str();
}

const unsigned scale = (1 << 23) - 1;

inline unsigned scaleSignal(double sig)
{
    double f = ((sig * 0.5) + 0.5) * scale;
    unsigned u = (unsigned)f << 8;
    return u;
}

inline unsigned mstosamps(double ms)
{
    return (unsigned)(ms * samplesPerMs);
}
