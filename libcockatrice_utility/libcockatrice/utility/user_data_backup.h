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

namespace UserDataBackup
{

/**
 * @brief Recursively copies @p sourceDir into @p destDir, creating it as needed.
 * @returns true when every file was copied (or there was nothing to copy).
 */
bool copyDirectoryRecursively(const QString &sourceDir, const QString &destDir);

/**
 * @brief Copies an at-risk path (file or directory) to a timestamped folder under @p backupRoot.
 * @param sourcePath file or directory holding user data (must exist).
 * @param backupRoot directory OUTSIDE the install tree that will hold timestamped backups.
 * @param backupLocationOut receives the created backup file or directory on success.
 * @returns true on success, false when the source is missing or nothing could be copied.
 */
bool backupAtRiskPath(const QString &sourcePath, const QString &backupRoot, QString *backupLocationOut = nullptr);

/**
 * @brief Default backup root guaranteed to live outside the install directory.
 */
QString defaultBackupRoot();

} // namespace UserDataBackup

#endif // USER_DATA_BACKUP_H
