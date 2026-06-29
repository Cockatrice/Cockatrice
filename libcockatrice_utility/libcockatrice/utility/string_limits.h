#ifndef STRING_LIMITS_H
#define STRING_LIMITS_H

#include <QString>
#include <algorithm>
#include <string>

// max size for short strings, like names and things that are generally a single phrase
constexpr int MAX_NAME_LENGTH = 0xff;
// max size for chat messages and text contents
constexpr int MAX_TEXT_LENGTH = 0xfff;
// max size for deck files and pictures
constexpr int MAX_FILE_LENGTH = 0x1fffff; // about 2 megabytes

// optimized functions to get qstrings that are at most that long
inline QString nameFromStdString(const std::string &_string)
{
    return QString::fromUtf8(_string.data(), std::min(int(_string.size()), MAX_NAME_LENGTH));
}
inline QString textFromStdString(const std::string &_string)
{
    return QString::fromUtf8(_string.data(), std::min(int(_string.size()), MAX_TEXT_LENGTH));
}
inline QString fileFromStdString(const std::string &_string)
{
    return QString::fromUtf8(_string.data(), std::min(int(_string.size()), MAX_FILE_LENGTH));
}

#endif // STRING_LIMITS_H
