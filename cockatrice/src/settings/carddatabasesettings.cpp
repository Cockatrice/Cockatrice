#include "carddatabasesettings.h"

CardDatabaseSettings::CardDatabaseSettings(QString settingPath, QObject *parent)
    : SettingsManager(settingPath+"cardDatabase.ini", parent)
{
}

void CardDatabaseSettings::setSortKey(QString shortName, unsigned int sortKey)
{
    setValue(sortKey,"sortkey", "sets", shortName);
}

void CardDatabaseSettings::setEnabled(QString shortName, bool enabled)
{
    setValue(enabled, "enabled", "sets", shortName);
}

void CardDatabaseSettings::setIsKnown(QString shortName, bool isknown)
{
    setValue(isknown, "isknown", "sets", shortName);
}

unsigned int CardDatabaseSettings::getSortKey(QString shortName)
{
    return getValue("sortkey", "sets", shortName).toUInt();
}

bool CardDatabaseSettings::isEnabled(QString shortName)
{
    return getValue("enabled", "sets", shortName).toBool();
}

bool CardDatabaseSettings::isKnown(QString shortName)
{
    return getValue("isknown",  "sets", shortName).toBool();
}
