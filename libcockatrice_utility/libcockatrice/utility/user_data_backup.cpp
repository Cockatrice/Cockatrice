#include "user_data_backup.h"

#include <QDateTime>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>

namespace UserDataBackup
{

namespace
{

QString uniqueTargetPath(const QString &sessionDir, const QString &fileName)
{
    QDir dir(sessionDir);
    QString candidate = dir.absoluteFilePath(fileName);
    if (!QFileInfo::exists(candidate)) {
        return candidate;
    }
    const QFileInfo info(fileName);
    const QString base = info.completeBaseName();
    const QString suffix = info.suffix();
    for (int i = 2;; ++i) {
        QString numbered = suffix.isEmpty() ? QStringLiteral("%1 (%2)").arg(base).arg(i)
                                            : QStringLiteral("%1 (%2).%3").arg(base).arg(i).arg(suffix);
        candidate = dir.absoluteFilePath(numbered);
        if (!QFileInfo::exists(candidate)) {
            return candidate;
        }
    }
}

} // namespace

bool copyDirectoryRecursively(const QString &sourceDir, const QString &destDir)
{
    QDir source(sourceDir);
    if (!source.exists()) {
        return false;
    }
    if (!QDir().mkpath(destDir)) {
        return false;
    }

    bool ok = true;
    QDirIterator it(sourceDir, QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::Hidden,
                    QDirIterator::Subdirectories);
    while (it.hasNext()) {
        it.next();
        QString entryRel = source.relativeFilePath(it.filePath());
        QString target = QDir(destDir).absoluteFilePath(entryRel);
        if (it.fileInfo().isDir()) {
            if (!QDir().mkpath(target)) {
                ok = false;
            }
        } else {
            // Ensure the parent exists even if the iterator yields a file
            // before its directory entry.
            QDir().mkpath(QFileInfo(target).absolutePath());
            if (QFile::exists(target) && !QFile::remove(target)) {
                ok = false;
                continue;
            }
            if (!QFile::copy(it.filePath(), target)) {
                ok = false;
            }
        }
    }
    return ok;
}

bool createBackupSessionDir(const QString &backupRoot, QString *sessionDirOut)
{
    if (!QDir().mkpath(backupRoot)) {
        return false;
    }
    const QString timestamp = QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd-hhmmss-zzz"));
    const QDir root(backupRoot);
    QString sessionDir = root.absoluteFilePath(QStringLiteral("CockatriceDataBackup-") + timestamp);
    for (int i = 1; QFileInfo::exists(sessionDir); ++i) {
        sessionDir = root.absoluteFilePath(QStringLiteral("CockatriceDataBackup-%1-%2").arg(timestamp).arg(i));
    }
    if (!QDir().mkpath(sessionDir)) {
        return false;
    }
    if (sessionDirOut) {
        *sessionDirOut = sessionDir;
    }
    return true;
}

bool backupAtRiskPath(const QString &sourcePath, const QString &backupRoot, QString *backupLocationOut)
{
    QStringList locations;
    const bool ok = backupAtRiskPaths({sourcePath}, backupRoot, &locations, nullptr);
    if (!locations.isEmpty() && backupLocationOut) {
        *backupLocationOut = locations.first();
    }
    return ok;
}

bool backupAtRiskPaths(const QStringList &sourcePaths,
                       const QString &backupRoot,
                       QStringList *backupLocationsOut,
                       QString *sessionDirOut)
{
    QStringList existing;
    for (const QString &sourcePath : sourcePaths) {
        if (QFileInfo::exists(sourcePath)) {
            existing.append(sourcePath);
        }
    }
    if (existing.isEmpty()) {
        return false;
    }

    QString sessionDir;
    if (!createBackupSessionDir(backupRoot, &sessionDir)) {
        return false;
    }
    if (sessionDirOut) {
        *sessionDirOut = sessionDir;
    }

    bool allOk = true;
    QStringList locations;
    for (const QString &sourcePath : existing) {
        const QFileInfo source(sourcePath);
        const QString backupLocation = uniqueTargetPath(sessionDir, source.fileName());
        bool ok;
        if (source.isDir()) {
            ok = copyDirectoryRecursively(sourcePath, backupLocation);
        } else {
            ok = QFile::copy(sourcePath, backupLocation);
        }
        // Report the location whenever a target was created, even for partial
        // copies, so callers can point the user at what was salvaged.
        if (QFileInfo::exists(backupLocation)) {
            locations.append(backupLocation);
        } else {
            // Total failure with no target created (e.g. QFile::copy failed).
            ok = false;
        }
        allOk = allOk && ok;
    }
    if (backupLocationsOut) {
        *backupLocationsOut = locations;
    }
    return allOk && !locations.isEmpty();
}

QString defaultBackupRoot()
{
    QString documents = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    if (!documents.isEmpty()) {
        return QDir(documents).absoluteFilePath("CockatriceBackups");
    }
    QString appData = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    if (!appData.isEmpty()) {
        return QDir(appData).absoluteFilePath("../CockatriceBackups");
    }
    return QDir::temp().absoluteFilePath("CockatriceBackups");
}

} // namespace UserDataBackup
