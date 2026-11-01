#ifndef COCKATRICE_REPLAY_DOWNLOAD_H
#define COCKATRICE_REPLAY_DOWNLOAD_H

#include <QByteArrayView>
#include <QString>

namespace ReplayDownload
{
// Creates or reuses one match directory beneath the selected download directory.
// Returns an empty path on failure and leaves the filesystem error in error.
QString prepareMatchDirectory(const QString &parentPath, int gameId, QString gameName, QString &error);

// Validates the wire payload and replaces the destination only after a complete
// successful write. Existing replays survive malformed data and filesystem errors.
bool save(const QString &filePath, QByteArrayView data, QString &error);
} // namespace ReplayDownload

#endif // COCKATRICE_REPLAY_DOWNLOAD_H
