#ifndef STRING_LIMITS_H
#define STRING_LIMITS_H

#include <QString>
#include <algorithm>
#include <string>

/** @brief Max size for short strings, like names and things that are generally a single phrase. */
constexpr int MAX_NAME_LENGTH = 0xff;
/** @brief Max size for chat messages and text contents. */
constexpr int MAX_TEXT_LENGTH = 0xfff;
/** @brief Max size for deck files and pictures (about 2 megabytes). */
constexpr int MAX_FILE_LENGTH = 0x1fffff;

/** @brief Returns a QString from a std::string, truncated to at most MAX_NAME_LENGTH bytes. */
inline QString nameFromStdString(const std::string &_string)
{
    return QString::fromUtf8(_string.data(), std::min(int(_string.size()), MAX_NAME_LENGTH));
}
/** @brief Returns a QString from a std::string, truncated to at most MAX_TEXT_LENGTH bytes. */
inline QString textFromStdString(const std::string &_string)
{
    return QString::fromUtf8(_string.data(), std::min(int(_string.size()), MAX_TEXT_LENGTH));
}
/** @brief Returns a QString from a std::string, truncated to at most MAX_FILE_LENGTH bytes. */
inline QString fileFromStdString(const std::string &_string)
{
    return QString::fromUtf8(_string.data(), std::min(int(_string.size()), MAX_FILE_LENGTH));
}

#endif // STRING_LIMITS_H
