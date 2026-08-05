#include "sound_settings.h"

SoundSettings::SoundSettings(const QString &settingPath, QObject *parent)
    : SettingsManager(settingPath + "sound.ini", "sound", QString(), parent)
{
}

bool SoundSettings::getSoundEnabled() const
{
    return getValue("enabled", QString(), QString(), false).toBool();
}

QString SoundSettings::getSoundThemeName() const
{
    return getValue("theme", QString(), QString()).toString();
}

int SoundSettings::getMasterVolume() const
{
    return getValue("masterVolume", QString(), QString(), 100).toInt();
}

void SoundSettings::setSoundEnabled(bool _soundEnabled)
{
    setValue(_soundEnabled, "enabled");
    emit soundEnabledChanged();
}

void SoundSettings::setSoundThemeName(const QString &_soundThemeName)
{
    setValue(_soundThemeName, "theme");
    emit soundThemeChanged();
}

void SoundSettings::setMasterVolume(int _masterVolume)
{
    setValue(_masterVolume, "masterVolume");
    emit masterVolumeChanged(_masterVolume);
}
