#ifndef GAME_SETTINGS_H
#define GAME_SETTINGS_H

#include "settings_manager.h"

#include <libcockatrice/interfaces/interface_game_settings_provider.h>

class GameSettings : public SettingsManager, public IGameSettingsProvider
{
    Q_OBJECT
    friend class SettingsCache;

public:
    [[nodiscard]] QString getGameDescription() const override;
    [[nodiscard]] int getMaxPlayers() const override;
    [[nodiscard]] QString getGameTypes() const override;
    [[nodiscard]] bool getOnlyBuddies() const override;
    [[nodiscard]] bool getOnlyRegistered() const override;
    [[nodiscard]] bool getSpectatorsAllowed() const override;
    [[nodiscard]] bool getSpectatorsNeedPassword() const override;
    [[nodiscard]] bool getSpectatorsCanTalk() const override;
    [[nodiscard]] bool getSpectatorsCanSeeEverything() const override;
    [[nodiscard]] bool getCreateGameAsSpectator() const override;
    [[nodiscard]] int getDefaultStartingLifeTotal() const override;
    [[nodiscard]] bool getShareDecklistsOnLoad() const override;
    [[nodiscard]] bool getRememberGameSettings() const override;
    [[nodiscard]] bool getLocalGameRememberSettings() const override;
    [[nodiscard]] int getLocalGameMaxPlayers() const override;
    [[nodiscard]] int getLocalGameStartingLifeTotal() const override;

    void setGameDescription(const QString &_gameDescription);
    void setMaxPlayers(int _maxPlayers);
    void setGameTypes(const QString &_gameTypes);
    void setOnlyBuddies(bool _onlyBuddies);
    void setOnlyRegistered(bool _onlyRegistered);
    void setSpectatorsAllowed(bool _spectatorsAllowed);
    void setSpectatorsNeedPassword(bool _spectatorsNeedPassword);
    void setSpectatorsCanTalk(bool _spectatorsCanTalk);
    void setSpectatorsCanSeeEverything(bool _spectatorsCanSeeEverything);
    void setCreateGameAsSpectator(bool _createGameAsSpectator);
    void setDefaultStartingLifeTotal(int _defaultStartingLifeTotal);
    void setShareDecklistsOnLoad(bool _shareDecklistsOnLoad);
    void setRememberGameSettings(bool _rememberGameSettings);
    void setLocalGameRememberSettings(bool value);
    void setLocalGameMaxPlayers(int value);
    void setLocalGameStartingLifeTotal(int value);

#ifdef SETTINGS_UNIT_TEST
public:
#else
private:
#endif
    explicit GameSettings(const QString &settingPath, QObject *parent = nullptr);
    GameSettings(const GameSettings & /*other*/);
};

#endif // GAME_SETTINGS_H
