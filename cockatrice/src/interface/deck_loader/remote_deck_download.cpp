#include "remote_deck_download.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QXmlStreamReader>

namespace RemoteDeckDownload
{
QString prepareDirectory(const QString &parentPath, const QString &name, QString &error)
{
    error.clear();
    bool invalidName = name.isEmpty() || name == QStringLiteral(".") || name == QStringLiteral("..") ||
                       name.contains(QLatin1Char('/')) || name.contains(QLatin1Char('\\')) ||
                       name.contains(QChar::Null) || name.contains(QLatin1Char(':'));
#ifdef Q_OS_WIN
    // Win32 strips trailing dots/spaces, which could merge two remote folders.
    invalidName = invalidName || name.endsWith(QLatin1Char('.')) || name.endsWith(QLatin1Char(' '));
    for (const auto character : name) {
        invalidName = invalidName || character.unicode() < 32 || QStringLiteral("<>\"|?*").contains(character);
    }
#endif
    if (invalidName) {
        error = QCoreApplication::translate("RemoteDeckDownload",
                                            "The remote folder name is not a local directory name: %1")
                    .arg(name);
        return {};
    }

    const QDir parent(parentPath);
    const QString directoryPath = parent.filePath(name);
    const QFileInfo existing(directoryPath);
    if (existing.isSymLink() || (existing.exists() && !existing.isDir()) ||
        (!existing.exists() && !parent.mkdir(name))) {
        error =
            QCoreApplication::translate("RemoteDeckDownload", "Could not create deck folder: %1").arg(directoryPath);
        return {};
    }
    return directoryPath;
}

std::optional<DeckList> parse(const QString &nativeString, QString &error)
{
    error.clear();
    QXmlStreamReader xml(nativeString);
    // Require the native format's actual root, while still accepting a
    // legitimate empty cockatrice_deck document.
    if (!xml.readNextStartElement() || xml.name() != QStringLiteral("cockatrice_deck")) {
        error =
            QCoreApplication::translate("RemoteDeckDownload", "The server did not provide a valid Cockatrice deck.");
        return std::nullopt;
    }
    DeckList deck;
    if (!deck.loadFromString_Native(nativeString)) {
        error = QCoreApplication::translate("RemoteDeckDownload", "The downloaded deck is invalid.");
        return std::nullopt;
    }
    return deck;
}
} // namespace RemoteDeckDownload
