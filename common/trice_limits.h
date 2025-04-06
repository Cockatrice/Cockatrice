#ifndef TRICE_LIMITS_H
#define TRICE_LIMITS_H

#include <QString>

// max size for short strings, like names and things that are generally a single phrase
constexpr int MAX_NAME_LENGTH = 0xff;
// max size for chat messages and text contents
constexpr int MAX_TEXT_LENGTH = 0xfff;
// max size for deck files and pictures
constexpr int MAX_FILE_LENGTH = 0x1fffff; // about 2 megabytes

constexpr uint MINIMUM_DIE_SIDES = 2;
constexpr uint MAXIMUM_DIE_SIDES = 1000000;
constexpr uint MINIMUM_DICE_TO_ROLL = 1;
constexpr uint MAXIMUM_DICE_TO_ROLL = 100;

// optimized functions to get qstrings that are at most that long
static inline QString nameFromStdString(const std::string &_string)
{
    return QString::fromUtf8(_string.data(), std::min(int(_string.size()), MAX_NAME_LENGTH));
}
static inline QString textFromStdString(const std::string &_string)
{
    return QString::fromUtf8(_string.data(), std::min(int(_string.size()), MAX_TEXT_LENGTH));
}
static inline QString fileFromStdString(const std::string &_string)
{
    return QString::fromUtf8(_string.data(), std::min(int(_string.size()), MAX_FILE_LENGTH));
}

#endif // TRICE_LIMITS_H
