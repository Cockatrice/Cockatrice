#ifndef COCKATRICE_INTERFACE_SOUND_SETTINGS_PROVIDER_H
#define COCKATRICE_INTERFACE_SOUND_SETTINGS_PROVIDER_H

#include <QString>

class ISoundSettingsProvider
{
public:
    virtual ~ISoundSettingsProvider() = default;

    [[nodiscard]] virtual bool getSoundEnabled() const = 0;
    [[nodiscard]] virtual QString getSoundThemeName() const = 0;
    [[nodiscard]] virtual int getMasterVolume() const = 0;
};

#endif // COCKATRICE_INTERFACE_SOUND_SETTINGS_PROVIDER_H
