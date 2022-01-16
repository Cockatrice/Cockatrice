// max sizes of strings used in the protocol
#ifndef STRINGSIZES_H
#define STRINGSIZES_H

#include <QString>
#include <QtMath>

// max size for short strings, like names and things that are generally a single phrase
constexpr int MAX_NAME_LENGTH = 0xff;
// max size for chat messages and text contents
constexpr int MAX_TEXT_LENGTH = 0xfff;
// max size for deck files and pictures
constexpr int MAX_FILE_LENGTH = 0xfffff; // about a megabyte

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

#endif // STRINGSIZES_H
