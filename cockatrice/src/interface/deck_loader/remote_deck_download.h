#ifndef COCKATRICE_REMOTE_DECK_DOWNLOAD_H
#define COCKATRICE_REMOTE_DECK_DOWNLOAD_H

#include <QModelIndexList>
#include <QString>
#include <libcockatrice/deck_list/deck_list.h>
#include <optional>

namespace RemoteDeckDownload
{
// A selected folder already includes its selected descendants. Preserve the
// order of independent selections without downloading those descendants twice.
QModelIndexList selectionRoots(const QModelIndexList &selection);

// Remote folders have no numeric ID prefix: preserve their exact names instead
// of folding distinct remote folders into the same sanitized local directory.
QString prepareDirectory(const QString &parentPath, const QString &name, QString &error);

// A failed or non-deck response must never become an empty saved deck.
std::optional<DeckList> parse(const QString &nativeString, QString &error);
} // namespace RemoteDeckDownload

#endif // COCKATRICE_REMOTE_DECK_DOWNLOAD_H
