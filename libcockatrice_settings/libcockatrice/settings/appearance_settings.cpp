#include "appearance_settings.h"

AppearanceSettings::AppearanceSettings(const QString &settingPath, QObject *parent)
    : SettingsManager(settingPath + "appearance.ini", "appearance", QString(), parent)
{
}

QString AppearanceSettings::getThemeName() const
{
    return getValue("themeName", QString(), QString()).toString();
}

void AppearanceSettings::setThemeName(const QString &_themeName)
{
    setValue(_themeName, "themeName");
    emit themeNameChanged();
}

bool AppearanceSettings::getStyleUserList() const
{
    return getValue("styleUserList", QString(), QString(), true).toBool();
}

void AppearanceSettings::setStyleUserList(bool _styleUserList)
{
    setValue(_styleUserList, "styleUserList");
    emit styleUserListChanged();
}

int AppearanceSettings::getMaxFontSize() const
{
    return getValue("maxFontSize", QString(), QString(), 12).toInt();
}

void AppearanceSettings::setMaxFontSize(int _max)
{
    setValue(_max, "maxFontSize");
}

QString AppearanceSettings::getHomeTabBackgroundSource() const
{
    return getValue("homeTabBackgroundSource", QString(), QString(), "themed").toString();
}

void AppearanceSettings::setHomeTabBackgroundSource(const QString &_backgroundSource)
{
    setValue(_backgroundSource, "homeTabBackgroundSource");
    emit homeTabBackgroundSourceChanged();
}

int AppearanceSettings::getHomeTabBackgroundShuffleFrequency() const
{
    return getValue("homeTabBackgroundShuffleFrequency", QString(), QString(), 0).toInt();
}

void AppearanceSettings::setHomeTabBackgroundShuffleFrequency(int _frequency)
{
    setValue(_frequency, "homeTabBackgroundShuffleFrequency");
    emit homeTabBackgroundShuffleFrequencyChanged();
}

bool AppearanceSettings::getHomeTabDisplayCardName() const
{
    return getValue("homeTabDisplayCardName", QString(), QString(), true).toBool();
}

void AppearanceSettings::setHomeTabDisplayCardName(bool _displayCardName)
{
    setValue(_displayCardName, "homeTabDisplayCardName");
    emit homeTabDisplayCardNameChanged();
}
