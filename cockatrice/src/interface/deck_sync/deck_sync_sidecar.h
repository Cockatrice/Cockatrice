/**
 * @file deck_sync_sidecar.h
 * @brief Per-profile local bookkeeping for deck collection sync.
 */

#ifndef DECK_SYNC_SIDECAR_H
#define DECK_SYNC_SIDECAR_H

#include <QMap>
#include <QString>

/** @brief Sync bookkeeping for one local deck file. */
struct DeckSyncRecord
{
    QString guid;            ///< Stable deck identity shared with the server.
    int remoteId = -1;       ///< Server row id, -1 when never uploaded.
    QString baseHash;        ///< Deck hash at the last successful sync.
    qint64 lastSyncSecs = 0; ///< Last sync time, seconds since Unix epoch.
    bool deleted = false;    ///< Local tombstone waiting to be pushed.
};

/**
 * @class DeckSyncSidecar
 * @brief Loads and stores sync records as JSON next to the deck folder.
 *
 * One sidecar file per DeckSyncProfile (named by profile key), mapping paths
 * relative to the deck folder to their last-synced state. Comparing the live
 * file hash against baseHash tells clean apart from locally modified.
 */
class DeckSyncSidecar
{
public:
    QString filePath; ///< Full path of the sidecar JSON file.

    explicit DeckSyncSidecar(const QString &_filePath);

    /** @brief Loads records; a missing file loads as empty and returns true. */
    bool load();
    /** @brief Stores records, creating parent folders as needed. */
    bool save() const;
    [[nodiscard]] bool contains(const QString &relativePath) const;
    /** @brief Returns the record, or a default record when unknown. */
    [[nodiscard]] DeckSyncRecord get(const QString &relativePath) const;
    void set(const QString &relativePath, const DeckSyncRecord &record);
    void remove(const QString &relativePath);
    [[nodiscard]] QMap<QString, DeckSyncRecord> records() const;

private:
    QMap<QString, DeckSyncRecord> recordMap;
};

#endif // DECK_SYNC_SIDECAR_H
