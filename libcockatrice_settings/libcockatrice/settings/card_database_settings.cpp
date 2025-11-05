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

unsigned int CardDatabaseSettings::getSortKey(QString shortName)
{
    return getValue("sortkey", "sets", std::move(shortName)).toUInt();
}

bool CardDatabaseSettings::isEnabled(QString shortName)
{
    return getValue("enabled", "sets", std::move(shortName)).toBool();
}

bool CardDatabaseSettings::isKnown(QString shortName)
{
    return getValue("isknown", "sets", std::move(shortName)).toBool();
}
