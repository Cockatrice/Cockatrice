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

const QString COMPLETE_MARKER = QStringLiteral("backup.complete");
const QString INCOMPLETE_MARKER = QStringLiteral("backup.incomplete");
const QString SOURCES_MANIFEST = QStringLiteral("backup.sources");

bool isMarkerFile(const QString &fileName)
{
    return fileName == COMPLETE_MARKER || fileName == INCOMPLETE_MARKER || fileName == SOURCES_MANIFEST;
}

bool pathsEqual(const QString &a, const QString &b)
{
#if defined(Q_OS_WIN) || defined(Q_OS_MACOS)
    return QDir::cleanPath(a).compare(QDir::cleanPath(b), Qt::CaseInsensitive) == 0;
#else
    return QDir::cleanPath(a) == QDir::cleanPath(b);
#endif
}

void writeCompletionMarker(const QString &sessionDir, bool complete)
{
    // Best effort: a missing marker only means isBackupSessionComplete()
    // reports the session as incomplete.
    QFile marker(QDir(sessionDir).absoluteFilePath(complete ? COMPLETE_MARKER : INCOMPLETE_MARKER));
    if (marker.open(QIODevice::WriteOnly | QIODevice::Text)) {
        marker.write(complete ? "complete\n" : "incomplete\n");
    }
}

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
        if (it.fileInfo().isSymLink()) {
            // Copy the link itself best-effort without dereferencing it. QDirIterator does not
            // follow symlinks here, so a symlink-to-directory must not take the isDir() branch
            // below (which would create an empty dir and silently drop the contents).
            QDir().mkpath(QFileInfo(target).absolutePath());
            if (QFile::exists(target) && !QFile::remove(target)) {
                ok = false;
                continue;
            }
            if (!QFile::copy(it.filePath(), target)) {
                ok = false;
            }
        } else if (it.fileInfo().isDir()) {
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
        if (backupLocationsOut) {
            backupLocationsOut->clear();
        }
        if (sessionDirOut) {
            sessionDirOut->clear();
        }
        return false;
    }

    QString sessionDir;
    if (!createBackupSessionDir(backupRoot, &sessionDir)) {
        if (backupLocationsOut) {
            backupLocationsOut->clear();
        }
        if (sessionDirOut) {
            sessionDirOut->clear();
        }
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
    if (locations.isEmpty()) {
        // Nothing salvaged: remove the session again so the empty directory
        // neither blocks a retry via the recent-session reuse nor evicts a
        // good session in pruneOldBackupSessions().
        QDir(sessionDir).removeRecursively();
        if (backupLocationsOut) {
            backupLocationsOut->clear();
        }
        if (sessionDirOut) {
            sessionDirOut->clear();
        }
        return false;
    }
    writeCompletionMarker(sessionDir, allOk);
    // Record which sources this session covers so callers can reuse it only
    // when it actually covers the current source set (see sessionCoversSources).
    QFile manifest(QDir(sessionDir).absoluteFilePath(SOURCES_MANIFEST));
    if (manifest.open(QIODevice::WriteOnly | QIODevice::Text)) {
        for (const QString &sourcePath : existing) {
            manifest.write(QDir::cleanPath(sourcePath).toUtf8() + "\n");
        }
    }
    if (backupLocationsOut) {
        *backupLocationsOut = locations;
    }
    return allOk;
}

qint64 estimateBackupSize(const QStringList &sourcePaths, qint64 capBytes)
{
    qint64 total = 0;
    for (const QString &sourcePath : sourcePaths) {
        const QFileInfo source(sourcePath);
        if (!source.exists()) {
            continue;
        }
        if (source.isDir()) {
            QDirIterator it(sourcePath, QDir::Files | QDir::Hidden | QDir::NoDotAndDotDot,
                            QDirIterator::Subdirectories);
            while (it.hasNext()) {
                it.next();
                total += it.fileInfo().size();
                if (total >= capBytes) {
                    return total;
                }
            }
        } else {
            total += source.size();
            if (total >= capBytes) {
                return total;
            }
        }
    }
    return total;
}

QString defaultBackupRoot()
{
    QString documents = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    if (!documents.isEmpty()) {
        return QDir(documents).absoluteFilePath("CockatriceBackups");
    }
    QString appData = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    if (!appData.isEmpty()) {
        return QDir::cleanPath(QDir(appData).absoluteFilePath("../CockatriceBackups"));
    }
    return QDir::temp().absoluteFilePath("CockatriceBackups");
}

QStringList listBackupSessions(const QString &backupRoot)
{
    QDir root(backupRoot);
    if (!root.exists()) {
        return {};
    }
    const QStringList names = root.entryList(QStringList() << QStringLiteral("CockatriceDataBackup-*"),
                                             QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name | QDir::Reversed);
    QStringList sessions;
    sessions.reserve(names.size());
    for (const QString &name : names) {
        sessions.append(root.absoluteFilePath(name));
    }
    return sessions;
}

QString findRecentCoveringBackupSession(const QString &backupRoot, const QStringList &sourcePaths, int maxAgeHours)
{
    const QStringList sessions = listBackupSessions(backupRoot);
    const QDateTime now = QDateTime::currentDateTime();
    for (const QString &session : sessions) {
        const QFileInfo info(session);
        if (!info.exists()) {
            continue;
        }
        if (info.lastModified().secsTo(now) >= static_cast<qint64>(maxAgeHours) * 3600) {
            continue;
        }
        if (listSessionBackupLocations(session).isEmpty()) {
            continue;
        }
        if (!sessionCoversSources(session, sourcePaths)) {
            continue;
        }
        return session;
    }
    return {};
}

QString findRecentUsableBackupSession(const QString &backupRoot, const QStringList &sourcePaths, int maxAgeHours)
{
    if (sourcePaths.isEmpty()) {
        return {};
    }
    const QStringList sessions = listBackupSessions(backupRoot);
    const QDateTime now = QDateTime::currentDateTime();
    for (const QString &session : sessions) {
        const QFileInfo info(session);
        if (!info.exists()) {
            continue;
        }
        if (info.lastModified().secsTo(now) >= static_cast<qint64>(maxAgeHours) * 3600) {
            continue;
        }
        if (listSessionBackupLocations(session).isEmpty()) {
            continue;
        }
        if (!isBackupSessionComplete(session)) {
            continue;
        }
        if (!sessionCoversSources(session, sourcePaths)) {
            continue;
        }
        return session;
    }
    return {};
}

bool isBackupSessionComplete(const QString &sessionDir)
{
    const QDir session(sessionDir);
    if (!session.exists()) {
        return false;
    }
    return QFileInfo::exists(session.absoluteFilePath(COMPLETE_MARKER));
}

void pruneOldBackupSessions(const QString &backupRoot, int keepCount)
{
    const QStringList sessions = listBackupSessions(backupRoot);
    QStringList kept;
    kept.reserve(sessions.size());
    for (const QString &session : sessions) {
        // Empty sessions are junk from failed attempts: drop them outright so
        // they can never evict a good session from the kept set.
        if (listSessionBackupLocations(session).isEmpty()) {
            QDir(session).removeRecursively();
            continue;
        }
        if (kept.size() < keepCount) {
            kept.append(session);
            continue;
        }
        QDir(session).removeRecursively();
    }
}

QStringList listSessionBackupLocations(const QString &sessionDir)
{
    const QDir session(sessionDir);
    if (!session.exists()) {
        return {};
    }
    const QStringList entries = session.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::Hidden);
    QStringList locations;
    locations.reserve(entries.size());
    for (const QString &entry : entries) {
        if (isMarkerFile(entry)) {
            continue;
        }
        locations.append(session.absoluteFilePath(entry));
    }
    return locations;
}

bool sessionCoversSources(const QString &sessionDir, const QStringList &sourcePaths)
{
    QFile manifest(QDir(sessionDir).absoluteFilePath(SOURCES_MANIFEST));
    if (!manifest.open(QIODevice::ReadOnly | QIODevice::Text)) {
        // No readable manifest (e.g. the write failed): coverage cannot be
        // verified, so report not-covered and let the caller take a fresh copy.
        return false;
    }
    QStringList recorded;
    while (!manifest.atEnd()) {
        const QString line = QString::fromUtf8(manifest.readLine()).trimmed();
        if (!line.isEmpty()) {
            recorded.append(line);
        }
    }
    for (const QString &source : sourcePaths) {
        bool found = false;
        for (const QString &entry : recorded) {
            if (pathsEqual(source, entry)) {
                found = true;
                break;
            }
        }
        if (!found) {
            return false;
        }
    }
    return true;
}

} // namespace UserDataBackup
