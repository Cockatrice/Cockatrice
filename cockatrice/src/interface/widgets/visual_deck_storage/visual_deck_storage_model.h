/**
 * @file visual_deck_storage_model.h
 * @ingroup VisualDeckStorageWidgets
 * @brief Source model for the Visual Deck Storage: the deck files on disk.
 *
 * The model owns the deck metadata (name, tags, color identity, banner card,
 * modification times) and the parsed deck list, loading each deck file in the
 * background. Views read through the roles or the direct accessors, and all
 * mutations (rename, tags, banner card, delete, conversion) go through this
 * class so the view layer never touches the filesystem directly.
 */

#ifndef VISUAL_DECK_STORAGE_MODEL_H
#define VISUAL_DECK_STORAGE_MODEL_H

#include "../../deck_loader/loaded_deck.h"

#include <QAbstractListModel>
#include <QDateTime>
#include <QList>
#include <QStringList>
#include <libcockatrice/deck_list/deck_list.h>
#include <optional>

namespace VisualDeckStorageRoles
{
/**
 * @brief Custom roles exposed by the VisualDeckStorageModel.
 */
enum
{
    FilePathRole = Qt::UserRole + 1, /**< Absolute file path of the deck. */
    RelativeFilePathRole,            /**< File path relative to the deck folder. */
    FolderPathRole,                  /**< Directory of the deck relative to the deck folder ("" for root). */
    DisplayNameRole,                 /**< Deck name, or the file name if the deck has no name. */
    TagsRole,                        /**< The deck's tags. */
    ColorIdentityRole,               /**< The deck's color identity (WUBRG order). */
    LastModifiedRole,                /**< QDateTime of the deck file's last modification. */
    LastLoadedRole,                  /**< QDateTime when the deck was last loaded from the file. */
    BannerCardNameRole,              /**< Name of the deck's banner card. */
    BannerCardProviderIdRole,        /**< Provider id of the deck's banner card. */
    /**
     * @brief Whether the row passes the proxy's current search / tag / color filters.
     *
     * Not served by this model, but by VisualDeckStorageSortFilterProxyModel on top
     * of it. Declared here so every role read through a proxy index stays unique.
     */
    FilterMatchRole
};
} // namespace VisualDeckStorageRoles

/**
 * @brief One deck file as seen by the Visual Deck Storage.
 *
 * The metadata is computed once when the deck loads and refreshed on reloads
 * and mutations, so filters and sorts never re-read the file from disk.
 */
struct DeckPreviewData
{
    QString filePath;            ///< Absolute file path.
    QString relativeFilePath;    ///< File path relative to the deck folder.
    QString folderPath;          ///< Directory relative to the deck folder ("" for the deck folder itself).
    QString deckName;            ///< The deck name as stored in the file (may be empty).
    QString displayName;         ///< Deck name, or the file name if the deck has no name.
    QStringList tags;            ///< The deck's tags.
    QString colorIdentity;       ///< The deck's color identity in WUBRG order.
    QDateTime lastModified;      ///< File modification time at last check.
    QDateTime lastLoaded;        ///< When the deck was last loaded from the file.
    CardRef bannerCard;          ///< The deck's banner card (name + provider id).
    LoadedDeck deck;             ///< The parsed deck; empty until the file has been loaded.
    bool loadSucceeded = false;  ///< Whether the deck file finished loading successfully.
    bool loadInProgress = false; ///< Whether the deck file is currently being loaded.
};

/**
 * @brief One finished background deck load that has not been applied to the model yet.
 */
struct PendingDeckLoad
{
    QString filePath;       ///< Identifies the row the result belongs to.
    int generation;         ///< Scan generation the load was started in.
    bool ok = false;        ///< Whether the file parsed successfully.
    LoadedDeck deck;        ///< The parsed deck, valid when ok.
    QDateTime lastModified; ///< File modification time at load, valid when ok.
    QString colorIdentity;  ///< WUBRG color identity computed off the UI thread, valid when ok.
};

/**
 * @brief The list model backing the Visual Deck Storage widget tree.
 *
 * Rows are in filesystem scan order; ordering and filtering are handled by
 * VisualDeckStorageSortFilterProxyModel on top of this model. The proxy keeps
 * every row and exposes each row's filter result through its FilterMatchRole.
 */
class VisualDeckStorageModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit VisualDeckStorageModel(QObject *parent = nullptr);

    /// @name Qt model overrides
    ///@{
    [[nodiscard]] int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;
    ///@}

    /**
     * @brief Sets the folder to scan for deck files and starts (re)loading.
     * Clears the model immediately (modelReset), then populates it asynchronously
     * as the background scan discovers deck files.
     */
    void setDeckPath(const QString &path);

    /**
     * @brief Re-scans the current deck folder, reloading every deck file.
     */
    void refresh();

    [[nodiscard]] QString getDeckPath() const
    {
        return deckPath;
    }

    /**
     * @brief The relative paths of all subdirectories of the deck folder, one level at a time.
     * Used by the view to build the folder tree. Sorted for deterministic order.
     */
    [[nodiscard]] QStringList getFolderPaths() const
    {
        return folderPaths;
    }

    /// @name Data accessors
    ///@{
    [[nodiscard]] const DeckPreviewData &dataForRow(int row) const;
    [[nodiscard]] const LoadedDeck &deckForRow(int row) const;
    [[nodiscard]] int rowForFilePath(const QString &filePath) const;
    ///@}

    /// @name Mutations (persist to disk and update the row)
    ///@{
    bool renameDeck(int row, const QString &newName);
    bool renameFile(int row, const QString &newBaseName);
    bool deleteFile(int row);
    bool setTags(int row, const QStringList &tags);
    bool setBannerCard(int row, const CardRef &cardRef);
    bool convertToCockatriceFormat(int row);
    bool reloadIfModified(int row);
    ///@}

signals:
    /**
     * @brief Emitted when a deck file finishes loading.
     * @param row The row of the deck that finished loading.
     */
    void deckLoaded(int row);

    /**
     * @brief Emitted when a deck's file path changes (rename file, conversion).
     * @param oldFilePath The previous file path.
     * @param newFilePath The new file path.
     */
    void deckFilePathChanged(const QString &oldFilePath, const QString &newFilePath);

private:
    void startScan();
    void beginLoad(int row);
    static void recomputeDeckMetadata(DeckPreviewData &data, bool recomputeColorIdentity = true);
    void schedulePendingLoadDrain();

private slots:
    void drainPendingLoads();

private:
    void setFilePathForRow(int row, const QString &newFilePath);

    QString deckPath;
    QList<DeckPreviewData> decks;
    QStringList folderPaths;               ///< All subdirectories of the deck folder, sorted.
    int scanGeneration = 0;                ///< Bumped on every scan so stale results are ignored.
    QVector<PendingDeckLoad> pendingLoads; ///< Finished background loads waiting to be applied.
    bool drainScheduled = false;           ///< Whether a queued drain pass is already pending.
};

#endif // VISUAL_DECK_STORAGE_MODEL_H
