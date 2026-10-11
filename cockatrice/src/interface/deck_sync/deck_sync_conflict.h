/**
 * @file deck_sync_conflict.h
 * @brief Conflict payload between the sync engine and the conflict dialog.
 */

#ifndef DECK_SYNC_CONFLICT_H
#define DECK_SYNC_CONFLICT_H

#include <QString>

/** @brief How a both-sides-changed deck is resolved. */
enum DeckSyncResolutionAction
{
    DeckSyncKeepMine,     ///< Upload the local deck, discarding the server version.
    DeckSyncKeepServer,   ///< Download the server deck, discarding local changes.
    DeckSyncKeepBoth,     ///< Keep the local deck and store the server version aside.
    DeckSyncUseMerge,     ///< Upload the auto-merged deck (only when mergeable).
    DeckSyncSkipConflict, ///< Leave both sides untouched for this run.
};

/** @brief Sync direction for one run. */
enum class DeckSyncMode
{
    TwoWay,       ///< Mirror both sides (default).
    UploadOnly,   ///< Push local content; never download or propagate deletes.
    DownloadOnly, ///< Take server content; never upload or push deletes.
    Separate,     ///< First-sync verdict only: keep this profile out of sync.
};

/** @brief One deck that changed both locally and on the server. */
struct DeckSyncConflict
{
    QString relativePath;      ///< Local deck path relative to the deck folder.
    QString localAbsolutePath; ///< Absolute local file path.
    QString serverDeckString;  ///< Server deck content in native format.
    QString baseDeckString;    ///< Last-synced content, empty when no snapshot exists.
    bool hasBase = false;      ///< False when the base snapshot is missing.
    qint64 localSecs = 0;      ///< Local modification time, seconds since epoch.
    qint64 serverSecs = 0;     ///< Server modification time, seconds since epoch.
};

/** @brief The dialog verdict for one DeckSyncConflict. */
struct DeckSyncResolution
{
    DeckSyncResolutionAction action = DeckSyncKeepMine;
    bool rememberSession = false; ///< Reuse this verdict for later conflicts this run.
    bool rememberAlways = false;  ///< Store this verdict as the conflict default setting.
};

#endif // DECK_SYNC_CONFLICT_H
