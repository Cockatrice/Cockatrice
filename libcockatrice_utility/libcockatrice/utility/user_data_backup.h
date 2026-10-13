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

/**
 * @brief Recursively copies @p sourceDir into @p destDir, creating it as needed.
 * @returns true when every file was copied (or there was nothing to copy).
 * Symlinks are copied best-effort via QFile::copy; a dangling link fails that entry only.
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
 * @brief Copies an at-risk path (file or directory) to a timestamped folder under @p backupRoot.
 * @param sourcePath file or directory holding user data (must exist).
 * @param backupRoot directory OUTSIDE the install tree that will hold timestamped backups.
 * @param backupLocationOut receives the created backup file or directory whenever a target
 * was created, even when the copy was only partial (i.e. also on failure with a partial copy).
 * @returns true when the whole source was copied, false when the source is missing or
 * any entry failed. A false return with a non-empty @p backupLocationOut means a partial
 * backup exists at that location.
 */
bool backupAtRiskPath(const QString &sourcePath, const QString &backupRoot, QString *backupLocationOut = nullptr);

/**
 * @brief Copies several at-risk paths into a single timestamped session directory.
 * @param sourcePaths files or directories holding user data; missing entries are skipped.
 * @param backupRoot directory OUTSIDE the install tree that will hold timestamped backups.
 * @param backupLocationsOut receives one entry per source that produced a backup target
 * (including partial copies).
 * @param sessionDirOut optionally receives the shared session directory.
 * @returns true when every existing source was copied completely.
 */
bool backupAtRiskPaths(const QStringList &sourcePaths,
                       const QString &backupRoot,
                       QStringList *backupLocationsOut,
                       QString *sessionDirOut = nullptr);

/**
 * @brief Default backup root guaranteed to live outside the install directory.
 */
QString defaultBackupRoot();

} // namespace UserDataBackup

#endif // USER_DATA_BACKUP_H
