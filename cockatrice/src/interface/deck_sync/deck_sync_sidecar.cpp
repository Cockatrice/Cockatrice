#include "deck_sync_sidecar.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>

namespace
{
constexpr int SIDECAR_FORMAT_VERSION = 1;
} // namespace

DeckSyncSidecar::DeckSyncSidecar(const QString &_filePath) : filePath(_filePath)
{
}

QString DeckSyncSidecar::pathForDeckPath(const QString &deckPath, const QString &profileKey)
{
    return QDir(deckPath).filePath(".sync/" + profileKey + ".json");
}

bool DeckSyncSidecar::load()
{
    recordMap.clear();
    mode = MODE_LINKED;
    lastSyncSecs = 0;

    QFile file(filePath);
    if (!file.exists()) {
        return true;
    }
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll());
    if (!document.isObject() || !document.object().value("records").isObject()) {
        return false;
    }
    const QJsonValue modeValue = document.object().value("mode");
    mode = modeValue.isString() ? modeValue.toString() : MODE_LINKED;
    lastSyncSecs = static_cast<qint64>(document.object().value("lastSync").toDouble());
    const QJsonObject records = document.object().value("records").toObject();
    for (auto it = records.begin(); it != records.end(); ++it) {
        if (!it.value().isObject()) {
            continue;
        }
        const QJsonObject entry = it.value().toObject();
        DeckSyncRecord record;
        record.guid = entry.value("guid").toString();
        record.remoteId = entry.value("remoteId").toInt(-1);
        record.baseHash = entry.value("baseHash").toString();
        record.lastSyncSecs = static_cast<qint64>(entry.value("lastSync").toDouble());
        record.deleted = entry.value("deleted").toBool(false);
        recordMap.insert(it.key(), record);
    }
    return true;
}

bool DeckSyncSidecar::save() const
{
    QJsonObject records;
    for (auto it = recordMap.begin(); it != recordMap.end(); ++it) {
        QJsonObject entry;
        entry.insert("guid", it.value().guid);
        entry.insert("remoteId", it.value().remoteId);
        entry.insert("baseHash", it.value().baseHash);
        entry.insert("lastSync", static_cast<double>(it.value().lastSyncSecs));
        entry.insert("deleted", it.value().deleted);
        records.insert(it.key(), entry);
    }
    QJsonObject root;
    root.insert("version", SIDECAR_FORMAT_VERSION);
    root.insert("mode", mode);
    root.insert("lastSync", static_cast<double>(lastSyncSecs));
    root.insert("records", records);

    QDir().mkpath(QFileInfo(filePath).absolutePath());
    QSaveFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    file.write(QJsonDocument(root).toJson(QJsonDocument::Compact));
    return file.commit();
}

bool DeckSyncSidecar::contains(const QString &relativePath) const
{
    return recordMap.contains(relativePath);
}

DeckSyncRecord DeckSyncSidecar::get(const QString &relativePath) const
{
    return recordMap.value(relativePath, DeckSyncRecord());
}

void DeckSyncSidecar::set(const QString &relativePath, const DeckSyncRecord &record)
{
    recordMap.insert(relativePath, record);
}

void DeckSyncSidecar::remove(const QString &relativePath)
{
    recordMap.remove(relativePath);
}

QMap<QString, DeckSyncRecord> DeckSyncSidecar::records() const
{
    return recordMap;
}
