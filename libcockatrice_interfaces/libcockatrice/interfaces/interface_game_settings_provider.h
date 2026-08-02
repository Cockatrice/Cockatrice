#ifndef COCKATRICE_INTERFACE_GAME_SETTINGS_PROVIDER_H
#define COCKATRICE_INTERFACE_GAME_SETTINGS_PROVIDER_H

#include <QString>

class IGameSettingsProvider
{
public:
    virtual ~IGameSettingsProvider() = default;

    [[nodiscard]] virtual QString getGameDescription() const = 0;
    [[nodiscard]] virtual int getMaxPlayers() const = 0;
    [[nodiscard]] virtual QString getGameTypes() const = 0;
    [[nodiscard]] virtual bool getOnlyBuddies() const = 0;
    [[nodiscard]] virtual bool getOnlyRegistered() const = 0;
    [[nodiscard]] virtual bool getSpectatorsAllowed() const = 0;
    [[nodiscard]] virtual bool getSpectatorsNeedPassword() const = 0;
    [[nodiscard]] virtual bool getSpectatorsCanTalk() const = 0;
    [[nodiscard]] virtual bool getSpectatorsCanSeeEverything() const = 0;
    [[nodiscard]] virtual bool getCreateGameAsSpectator() const = 0;
    [[nodiscard]] virtual int getDefaultStartingLifeTotal() const = 0;
    [[nodiscard]] virtual bool getShareDecklistsOnLoad() const = 0;
    [[nodiscard]] virtual bool getRememberGameSettings() const = 0;
    [[nodiscard]] virtual bool getLocalGameRememberSettings() const = 0;
    [[nodiscard]] virtual int getLocalGameMaxPlayers() const = 0;
    [[nodiscard]] virtual int getLocalGameStartingLifeTotal() const = 0;
};

#endif // COCKATRICE_INTERFACE_GAME_SETTINGS_PROVIDER_H
