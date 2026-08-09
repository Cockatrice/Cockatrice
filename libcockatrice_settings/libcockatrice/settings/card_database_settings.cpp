#include "card_database_settings.h"

#include <QMutexLocker>

CardDatabaseSettings::CardDatabaseSettings(const QString &settingPath, QObject *parent)
    : SettingsManager(settingPath + "cardDatabase.ini", QString(), QString(), parent)
{
}

void CardDatabaseSettings::setSortKey(QString shortName, unsigned int sortKey)
{
    setValue(sortKey, "sortKey", "sets", shortName);
    QMutexLocker lock(&setOptionsMutex);
    ensureSetOptionsLoaded();
    setOptionsCache[shortName].sortKey = sortKey;
}

void CardDatabaseSettings::setEnabled(QString shortName, bool enabled)
{
    setValue(enabled, "enabled", "sets", shortName);
    QMutexLocker lock(&setOptionsMutex);
    ensureSetOptionsLoaded();
    setOptionsCache[shortName].enabled = enabled;
}

void CardDatabaseSettings::setIsKnown(QString shortName, bool isknown)
{
    setValue(isknown, "isKnown", "sets", shortName);
    QMutexLocker lock(&setOptionsMutex);
    ensureSetOptionsLoaded();
    setOptionsCache[shortName].isKnown = isknown;
}

void CardDatabaseSettings::ensureSetOptionsLoaded() const
{
    if (setOptionsLoaded) {
        return;
    }
    QSettings settings(getSettings());
    settings.beginGroup("sets");
    const QStringList groups = settings.childGroups();
    for (const QString &group : groups) {
        settings.beginGroup(group);
        SetOptions &o = setOptionsCache[group];
        o.sortKey = settings.value("sortKey", 0).toUInt();
        o.enabled = settings.value("enabled", true).toBool();
        o.isKnown = settings.value("isKnown", true).toBool();
        settings.endGroup();
    }
    setOptionsLoaded = true;
}

unsigned int CardDatabaseSettings::getSortKey(QString shortName) const
{
    QMutexLocker lock(&setOptionsMutex);
    ensureSetOptionsLoaded();
    return setOptionsCache.value(shortName).sortKey;
}

bool CardDatabaseSettings::isEnabled(QString shortName) const
{
    QMutexLocker lock(&setOptionsMutex);
    ensureSetOptionsLoaded();
    return setOptionsCache.value(shortName).enabled;
}

bool CardDatabaseSettings::isKnown(QString shortName) const
{
    QMutexLocker lock(&setOptionsMutex);
    ensureSetOptionsLoaded();
    return setOptionsCache.value(shortName).isKnown;
}

ICardSetPriorityController::SetOptions CardDatabaseSettings::getSetOptions(QString shortName) const
{
    QMutexLocker lock(&setOptionsMutex);
    ensureSetOptionsLoaded();
    return setOptionsCache.value(shortName);
}

void CardDatabaseSettings::saveSets(const QVector<ICardSetPriorityController::SetSaveData> &data)
{
    batchWrite([&](QSettings &s) {
        s.beginGroup("sets");
        for (const auto &entry : data) {
            s.beginGroup(entry.shortName);
            s.setValue("sortKey", entry.sortKey);
            s.setValue("enabled", entry.enabled);
            s.endGroup();
        }
        s.endGroup();
    });

    QMutexLocker lock(&setOptionsMutex);
    ensureSetOptionsLoaded();
    for (const auto &entry : data) {
        SetOptions &o = setOptionsCache[entry.shortName];
        o.sortKey = entry.sortKey;
        o.enabled = entry.enabled;
    }
}