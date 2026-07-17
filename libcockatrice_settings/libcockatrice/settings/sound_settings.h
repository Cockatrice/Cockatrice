#ifndef SOUND_SETTINGS_H
#define SOUND_SETTINGS_H

#include "settings_manager.h"

#include <libcockatrice/interfaces/interface_sound_settings_provider.h>

class SoundSettings : public SettingsManager, public ISoundSettingsProvider
{
    Q_OBJECT
    friend class SettingsCache;

public:
    [[nodiscard]] bool getSoundEnabled() const override;
    [[nodiscard]] QString getSoundThemeName() const override;
    [[nodiscard]] int getMasterVolume() const override;

    void setSoundEnabled(bool _soundEnabled);
    void setSoundThemeName(const QString &_soundThemeName);
    void setMasterVolume(int _masterVolume);

signals:
    void soundEnabledChanged();
    void soundThemeChanged();
    void masterVolumeChanged(int value);

#ifdef SETTINGS_UNIT_TEST
public:
#else
private:
#endif
    explicit SoundSettings(const QString &settingPath, QObject *parent = nullptr);
    SoundSettings(const SoundSettings & /*other*/);
};

#endif // SOUND_SETTINGS_H
