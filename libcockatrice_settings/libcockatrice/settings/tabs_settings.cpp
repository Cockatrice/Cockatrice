#include "tabs_settings.h"

TabsSettings::TabsSettings(const QString &settingPath, QObject *parent)
    : SettingsManager(settingPath + "tabs.ini", "tabs", QString(), parent)
{
}

bool TabsSettings::getTabVisualDeckStorageOpen() const
{
    return getValue("visualDeckStorage", QString(), QString(), true).toBool();
}

bool TabsSettings::getTabServerOpen() const
{
    return getValue("server", QString(), QString(), true).toBool();
}

bool TabsSettings::getTabAccountOpen() const
{
    return getValue("account", QString(), QString(), true).toBool();
}

bool TabsSettings::getTabDeckStorageOpen() const
{
    return getValue("deckStorage", QString(), QString(), true).toBool();
}

bool TabsSettings::getTabReplaysOpen() const
{
    return getValue("replays", QString(), QString(), true).toBool();
}

bool TabsSettings::getTabAdminOpen() const
{
    return getValue("admin", QString(), QString(), true).toBool();
}

bool TabsSettings::getTabLogOpen() const
{
    return getValue("log", QString(), QString(), true).toBool();
}

void TabsSettings::setTabVisualDeckStorageOpen(bool value)
{
    setValue(value, "visualDeckStorage");
}

void TabsSettings::setTabServerOpen(bool value)
{
    setValue(value, "server");
}

void TabsSettings::setTabAccountOpen(bool value)
{
    setValue(value, "account");
}

void TabsSettings::setTabDeckStorageOpen(bool value)
{
    setValue(value, "deckStorage");
}

void TabsSettings::setTabReplaysOpen(bool value)
{
    setValue(value, "replays");
}

void TabsSettings::setTabAdminOpen(bool value)
{
    setValue(value, "admin");
}

void TabsSettings::setTabLogOpen(bool value)
{
    setValue(value, "log");
}
