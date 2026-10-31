/**
 * @file user_data_backup.h
 * @ingroup Utility
 * @brief Best-effort safety copies of user data that lives inside the install directory.
 *
 * Background: Windows uninstallers shipped in 3.1.0/3.1.1 ran a blanket
 * "RMDir /r $INSTDIR". For a portable install that deletes $INSTDIR\data
 * (decks, settings, pictures) along with the payload, and for a normal install
 * it deletes anything the user pointed into $INSTDIR. The fixed uninstaller
 * preserves foreign files, but the destructive one is already on disk on
 * affected machines, so the client keeps a safety copy outside the install
 * directory before warning the user. Detection of at-risk paths lives in
 * PathsSettings::pathsInsideDir; this helper only copies.
 */

#ifndef USER_DATA_BACKUP_H
#define USER_DATA_BACKUP_H

#include <QString>
#include <QStringList>

namespace UserDataBackup
{

/** @brief Above this total, the client skips the automatic startup backup and asks for a manual copy.
 * Kept small on purpose: the copy runs synchronously on the UI thread at startup. */
constexpr qint64 MAX_AUTO_BACKUP_BYTES = 200LL * 1024 * 1024;

/**
 * @brief Recursively copies @p sourceDir into @p destDir, creating it as needed.
 * @returns true when every file was copied (or there was nothing to copy).
 * Symlinks (file or directory) are copied best-effort via QFile::copy without
 * dereferencing them; a dangling link fails that entry only.
 */
bool copyDirectoryRecursively(const QString &sourceDir, const QString &destDir);

/**
 * @brief Creates a fresh timestamped session directory under @p backupRoot.
 * @param backupRoot directory OUTSIDE the install tree that will hold timestamped backups.
 * @param sessionDirOut receives the created directory on success.
 * @returns true on success. Millisecond timestamps plus a numeric suffix keep rapid
 * successive calls (e.g. one per installer phase) from sharing a directory.
 */
bool createBackupSessionDir(const QString &backupRoot, QString *sessionDirOut);

/**
 * @brief Copies several at-risk paths into a single timestamped session directory.
 * @param sourcePaths files or directories holding user data; missing entries are skipped.
 * @param backupRoot directory OUTSIDE the install tree that will hold timestamped backups.
 * @param backupLocationsOut receives one entry per source that produced a backup target
 * (including partial copies).
 * @param sessionDirOut optionally receives the shared session directory; set to empty
 * when nothing was backed up (all sources missing, or every copy failed outright).
 * @returns true when every existing source was copied completely.
 * A false return with a non-empty @p backupLocationsOut means a partial backup exists.
 * When nothing at all could be salvaged the session directory is removed again, so a
 * failed attempt never leaves an empty session behind that would block retries.
 * Completeness is recorded with a marker file; see isBackupSessionComplete().
 */
bool backupAtRiskPaths(const QStringList &sourcePaths,
                       const QString &backupRoot,
                       QStringList *backupLocationsOut,
                       QString *sessionDirOut = nullptr);

/**
 * @brief Estimates the total bytes that backing up @p sourcePaths would copy.
 * @param capBytes stop counting once the total exceeds this; the returned value is then
 * at least @p capBytes (used to skip the scan early on huge trees).
 * Missing entries contribute 0. Best effort only.
 */
qint64 estimateBackupSize(const QStringList &sourcePaths, qint64 capBytes = MAX_AUTO_BACKUP_BYTES);

/**
 * @brief Default backup root guaranteed to live outside the install directory.
 */
QString defaultBackupRoot();

/**
 * @brief Lists existing backup session directories under @p backupRoot, newest first.
 */
QStringList listBackupSessions(const QString &backupRoot);

/**
 * @brief Returns the newest session that is non-empty, complete and covering @p sourcePaths, or empty.
 *
 * An incomplete newest session (partial failure) or one covering a different
 * source set does not block reuse of an older good session.
 */
QString findRecentUsableBackupSession(const QString &backupRoot, const QStringList &sourcePaths, int maxAgeHours = 24);

/**
 * @brief Returns the newest non-empty session covering @p sourcePaths, regardless of completeness.
 *
 * Used as a fallback pointer when a fresh copy fails entirely, so the dialog can still
 * point at salvaged content. Completeness is left to the caller to report.
 */
QString
findRecentCoveringBackupSession(const QString &backupRoot, const QStringList &sourcePaths, int maxAgeHours = 24);

/**
 * @brief Reports whether @p sessionDir holds a complete backup.
 * @returns true when the session carries the complete marker, false otherwise
 * (partial backups and empty/missing sessions).
 */
bool isBackupSessionComplete(const QString &sessionDir);

/**
 * @brief Deletes oldest sessions keeping only the newest @p keepCount ones.
 * Empty sessions are junk from failed attempts and are always removed, so they
 * can never evict a good session from the kept set.
 */
void pruneOldBackupSessions(const QString &backupRoot, int keepCount = 5);

/**
 * @brief Lists the backup targets inside an existing session directory.
 * @returns absolute paths of the entries, for reuse messages without a new copy.
 * Completeness marker and source manifest files are internal bookkeeping and are never listed.
 */
QStringList listSessionBackupLocations(const QString &sessionDir);

/**
 * @brief Reports whether @p sessionDir was recorded as covering all of @p sourcePaths.
 * @returns true when the session's source manifest lists every entry of @p sourcePaths
 * (normalized via QDir::cleanPath, case-insensitive on Windows/macOS). Sessions
 * without a readable manifest return false so a changed source set always triggers
 * a fresh copy instead of reusing a backup that may not cover it.
 */
bool sessionCoversSources(const QString &sessionDir, const QStringList &sourcePaths);

} // namespace UserDataBackup

#endif // USER_DATA_BACKUP_H
