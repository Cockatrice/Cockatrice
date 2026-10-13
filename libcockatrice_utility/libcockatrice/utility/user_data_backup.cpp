#include "user_data_backup.h"

#include <QDateTime>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>

namespace UserDataBackup
{

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

bool backupAtRiskPath(const QString &sourcePath, const QString &backupRoot, QString *backupLocationOut)
{
    QFileInfo source(sourcePath);
    if (!source.exists()) {
        return false;
    }
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd-hhmmss");
    QString backupDir = QDir(backupRoot).absoluteFilePath("CockatriceDataBackup-" + timestamp);
    if (!QDir().mkpath(backupDir)) {
        return false;
    }

    bool ok;
    QString backupLocation;
    if (source.isDir()) {
        backupLocation = QDir(backupDir).absoluteFilePath(source.fileName());
        ok = copyDirectoryRecursively(sourcePath, backupLocation);
    } else {
        backupLocation = QDir(backupDir).absoluteFilePath(source.fileName());
        if (QFile::exists(backupLocation)) {
            QFile::remove(backupLocation);
        }
        ok = QFile::copy(sourcePath, backupLocation);
    }
    if (!ok) {
        return false;
    }
    if (backupLocationOut) {
        *backupLocationOut = backupLocation;
    }
    return true;
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
