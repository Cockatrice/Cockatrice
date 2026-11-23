#include "spoiler_file.h"

#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSaveFile>
#include <QXmlStreamReader>

namespace
{
QByteArray hashFile(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return {};
    }
    QCryptographicHash hash(QCryptographicHash::Sha256);
    return hash.addData(&file) ? hash.result() : QByteArray();
}
} // namespace

namespace SpoilerFile
{
QString validate(const QByteArray &data)
{
    QXmlStreamReader xml(data);
    if (!xml.readNextStartElement() || xml.name() != QStringLiteral("cockatrice_carddatabase")) {
        return QCoreApplication::translate(
            "SpoilerBackgroundUpdater",
            "The download is not a Cockatrice card database. The previous spoiler file was preserved.");
    }
    bool versionOk = false;
    const int version = xml.attributes().value(QStringLiteral("version")).toInt(&versionOk);
    if (!versionOk || (version != 3 && version != 4)) {
        return QCoreApplication::translate(
            "SpoilerBackgroundUpdater",
            "The spoiler database uses an unsupported format. The previous spoiler file was preserved.");
    }
    bool hasCards = false;
    int depth = 1;
    while (!xml.atEnd()) {
        const auto token = xml.readNext();
        if (token == QXmlStreamReader::StartElement) {
            ++depth;
            if (depth == 2 && xml.name() == QStringLiteral("cards")) {
                hasCards = true;
            }
        } else if (token == QXmlStreamReader::EndElement) {
            --depth;
        }
    }
    if (xml.hasError() || !hasCards) {
        return QCoreApplication::translate(
            "SpoilerBackgroundUpdater",
            "The spoiler database is incomplete or invalid. The previous spoiler file was preserved.");
    }
    return {};
}

SaveResult
save(const QString &targetPath, const QByteArray &data, const std::function<bool()> &isCurrent, QString &error)
{
    error = validate(data);
    if (!error.isEmpty()) {
        return SaveResult::Failed;
    }
    if (!isCurrent()) {
        error = QCoreApplication::translate("SpoilerBackgroundUpdater",
                                            "The spoiler update was cancelled because its settings changed.");
        return SaveResult::Failed;
    }
    if (hashFile(targetPath) == QCryptographicHash::hash(data, QCryptographicHash::Sha256)) {
        return SaveResult::Unchanged;
    }
    const QDir directory = QFileInfo(targetPath).dir();
    if (!directory.exists() && !directory.mkpath(QStringLiteral("."))) {
        error = QCoreApplication::translate("SpoilerBackgroundUpdater",
                                            "Could not create the spoiler database directory: %1")
                    .arg(directory.absolutePath());
        return SaveResult::Failed;
    }
    QSaveFile file(targetPath);
    file.setDirectWriteFallback(false);
    if (!file.open(QIODevice::WriteOnly) || file.write(data) != data.size() || !file.flush()) {
        error = QCoreApplication::translate(
                    "SpoilerBackgroundUpdater",
                    "Could not save the spoiler database to %1: %2. The previous file was preserved.")
                    .arg(targetPath, file.errorString());
        return SaveResult::Failed;
    }
    if (!isCurrent()) {
        file.cancelWriting();
        error = QCoreApplication::translate("SpoilerBackgroundUpdater",
                                            "The spoiler update was cancelled because its settings changed.");
        return SaveResult::Failed;
    }
    if (!file.commit()) {
        error = QCoreApplication::translate(
                    "SpoilerBackgroundUpdater",
                    "Could not replace the spoiler database at %1: %2. The previous file was preserved.")
                    .arg(targetPath, file.errorString());
        return SaveResult::Failed;
    }
    return SaveResult::Written;
}

} // namespace SpoilerFile
