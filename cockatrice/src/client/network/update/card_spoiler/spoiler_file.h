#ifndef COCKATRICE_SPOILER_FILE_H
#define COCKATRICE_SPOILER_FILE_H

#include <QByteArray>
#include <QString>
#include <functional>

namespace SpoilerFile
{
enum class SaveResult
{
    Failed,
    Unchanged,
    Written
};

QString validate(const QByteArray &data);
// Validate before opening the destination, compare complete files, and replace
// atomically. Check operation ownership both before writing and before committing.
SaveResult
save(const QString &targetPath, const QByteArray &data, const std::function<bool()> &isCurrent, QString &error);
} // namespace SpoilerFile

#endif // COCKATRICE_SPOILER_FILE_H
