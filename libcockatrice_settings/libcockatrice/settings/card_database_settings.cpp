#include "card_database_settings.h"

CardDatabaseSettings::CardDatabaseSettings(const QString &settingPath, QObject *parent)
    : SettingsManager(settingPath + "cardDatabase.ini", QString(), QString(), parent)
{
}

void CardDatabaseSettings::setSortKey(QString shortName, unsigned int sortKey)
{
    setValue(sortKey, "sortkey", "sets", std::move(shortName));
}

void CardDatabaseSettings::setEnabled(QString shortName, bool enabled)
{
    setValue(enabled, "enabled", "sets", std::move(shortName));
}

void CardDatabaseSettings::setIsKnown(QString shortName, bool isknown)
{
    setValue(isknown, "isknown", "sets", std::move(shortName));
}

unsigned int CardDatabaseSettings::getSortKey(QString shortName) const
{
    return getValue("sortkey", "sets", std::move(shortName)).toUInt();
}

bool CardDatabaseSettings::isEnabled(QString shortName) const
{
    return getValue("enabled", "sets", std::move(shortName)).toBool();
}

bool CardDatabaseSettings::isKnown(QString shortName) const
{
    return getValue("isknown", "sets", std::move(shortName)).toBool();
}

void CardDatabaseSettings::saveSets(const QVector<ICardSetPriorityController::SetSaveData> &data)
{
    batchWrite([&](QSettings &s) {
        s.beginGroup("sets");
        for (const auto &entry : data) {
            s.beginGroup(entry.shortName);
            s.setValue("sortkey", entry.sortKey);
            s.setValue("enabled", entry.enabled);
            s.endGroup();
        }
        s.endGroup();
    });
}