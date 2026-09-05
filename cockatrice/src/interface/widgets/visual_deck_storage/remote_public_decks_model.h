/**
 * @file remote_public_decks_model.h
 * @ingroup DeckStorageWidgets
 */

#ifndef REMOTE_PUBLIC_DECKS_MODEL_H
#define REMOTE_PUBLIC_DECKS_MODEL_H

#include "visual_deck_storage_sort_filter_proxy_model.h"

#include <QAbstractListModel>
#include <QDateTime>
#include <QList>
#include <QSet>
#include <QStringList>

class AbstractClient;
class CommandContainer;
class Response;
class ServerInfo_DeckStorage_Folder;
class ServerInfo_DeckStorage_TreeItem;

/**
 * @brief Flat, read-only list of the public decks published by another user.
 *
 * Fetches the target user's public decks via Command_DeckListOtherUser and
 * flattens the response tree into entries carrying the preview metadata stored
 * on the server (banner card name/provider and color identity). No deck list is
 * downloaded until the user actually opens a deck.
 *
 * Name and color-identity filtering is applied against this metadata, mirroring
 * the Visual Deck Storage's filter semantics, so the grid can be narrowed like
 * the local deck storage.
 */
class RemotePublicDecksModel : public QAbstractListModel
{
    Q_OBJECT

public:
    struct DeckEntry
    {
        int id = 0;
        QString name;
        QDateTime uploadTime;
        QString bannerCardName;
        QString bannerCardProvider;
        QString colorIdentity;
        QStringList tags;
    };

    /**
     * @brief The color identity filter mode, shared with the Visual Deck Storage.
     */
    using FilterMode = VisualDeckStorageSortFilterProxyModel::FilterMode;

    explicit RemotePublicDecksModel(AbstractClient *client, QObject *parent = nullptr);

    [[nodiscard]] int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    [[nodiscard]] QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    /** @brief Fetches the public decks of another user, replacing the current contents. */
    void refresh(const QString &userName);
    void clear();

    /** @brief Sets a case-insensitive substring filter on the deck name. */
    void setSearchText(const QString &text);

    /** @brief Sets the active color identity filter and mode. */
    void setColorFilter(FilterMode mode, const QSet<QChar> &colors);

    /** @brief Filters decks by required (`selected`) and forbidden (`excluded`) tags. */
    void setTagFilter(const QSet<QString> &selected, const QSet<QString> &excluded);

    /** @brief All tags present across all loaded decks, for building filter chips. */
    [[nodiscard]] QSet<QString> allTags() const;

    /** @brief The number of decks after filtering. */
    [[nodiscard]] int filteredCount() const
    {
        return visibleIndices.size();
    }

    /** @brief The number of decks before filtering. */
    [[nodiscard]] int totalCount() const
    {
        return decks.size();
    }

    /** @brief True while a refresh request is in flight and the grid has no data yet. */
    [[nodiscard]] bool isLoading() const
    {
        return loading;
    }

    [[nodiscard]] DeckEntry entryAt(int row) const;

signals:
    /** @brief Emitted when a refresh starts, completes, or fails (see loading()). */
    void loadingChanged(bool loading);

    /** @brief Emitted when the last refresh failed; contains a user-facing message. */
    void loadFailed(const QString &message);

private slots:
    void decksReceived(const Response &response, const CommandContainer &commandContainer);

private:
    void addFolder(const ServerInfo_DeckStorage_Folder &folder);
    void addTreeItem(const ServerInfo_DeckStorage_TreeItem &item);
    void rebuildVisibleIndices();
    void setLoading(bool value);

    AbstractClient *client;
    QList<DeckEntry> decks;
    QList<int> visibleIndices; ///< Row indices into `decks` that pass the current filters.
    bool loading = false;

    QString searchText;
    VisualDeckStorageSortFilterProxyModel::FilterMode colorFilterMode = VisualDeckStorageSortFilterProxyModel::Includes;
    QSet<QChar> activeColors;
    QSet<QString> includedTags;
    QSet<QString> excludedTags;
};

#endif // REMOTE_PUBLIC_DECKS_MODEL_H
