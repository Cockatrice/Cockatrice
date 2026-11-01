#include "replay_download.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QIODevice>
#include <QSaveFile>
#include <libcockatrice/protocol/pb/game_replay.pb.h>
#include <limits>

namespace ReplayDownload
{
QString prepareMatchDirectory(const QString &parentPath, int gameId, QString gameName, QString &error)
{
    error.clear();
    // Game names are display text, not paths. Use the same component on every
    // platform, including names containing separators or Windows-reserved characters.
    for (QChar &character : gameName) {
        if (character.unicode() < 32 || QStringLiteral("<>:\"/\\|?*").contains(character)) {
            character = QLatin1Char('_');
        }
    }
    while (gameName.endsWith(QLatin1Char('.')) || gameName.endsWith(QLatin1Char(' '))) {
        gameName.chop(1);
    }
    const QString directoryName =
        gameName.isEmpty() ? QString::number(gameId) : QStringLiteral("%1_%2").arg(gameId).arg(gameName);
    const QDir parent(parentPath);
    const QString directoryPath = parent.filePath(directoryName);
    const QFileInfo existing(directoryPath);
    // A pre-existing link must not redirect a remotely named match out of the
    // selected directory. Explicitly selected parent directories remain supported.
    if (existing.isSymLink() || (existing.exists() && !existing.isDir()) ||
        (!existing.exists() && !parent.mkdir(directoryName))) {
        error = QCoreApplication::translate("ReplayDownload", "Could not create replay folder: %1").arg(directoryPath);
        return {};
    }
    return directoryPath;
}

bool save(const QString &filePath, QByteArrayView data, QString &error)
{
    error.clear();
    GameReplay replay;
    if (data.isEmpty() || data.size() > std::numeric_limits<int>::max() ||
        !replay.ParseFromArray(data.data(), static_cast<int>(data.size()))) {
        error = QCoreApplication::translate("ReplayDownload", "The downloaded replay is invalid.");
        return false;
    }

    QSaveFile file(filePath);
    // Check the buffered write before committing: on Qt 6.4, commit() can
    // otherwise report success after a short write during the implicit flush.
    if (!file.open(QIODevice::WriteOnly) || file.write(data.data(), data.size()) != data.size() || !file.flush() ||
        !file.commit()) {
        error = QCoreApplication::translate("ReplayDownload", "Could not save replay to %1: %2")
                    .arg(filePath, file.errorString());
        return false;
    }
    return true;
}
} // namespace ReplayDownload
