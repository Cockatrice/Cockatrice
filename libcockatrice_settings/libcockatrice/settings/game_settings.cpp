#include "game_settings.h"

GameSettings::GameSettings(const QString &settingPath, QObject *parent)
    : SettingsManager(settingPath + "game.ini", QString(), QString(), parent)
{
}

QString GameSettings::getGameDescription() const
{
    return getValue("gameDescription", "game").toString();
}

int GameSettings::getMaxPlayers() const
{
    return getValue("maxPlayers", "game", QString(), 2).toInt();
}

QString GameSettings::getGameTypes() const
{
    return getValue("gameTypes", "game").toString();
}

bool GameSettings::getOnlyBuddies() const
{
    return getValue("onlyBuddies", "game").toBool();
}

bool GameSettings::getOnlyRegistered() const
{
    return getValue("onlyRegistered", "game").toBool();
}

bool GameSettings::getSpectatorsAllowed() const
{
    return getValue("spectatorsAllowed", "game").toBool();
}

bool GameSettings::getSpectatorsNeedPassword() const
{
    return getValue("spectatorsNeedPassword", "game").toBool();
}

bool GameSettings::getSpectatorsCanTalk() const
{
    return getValue("spectatorsCanTalk", "game").toBool();
}

bool GameSettings::getSpectatorsCanSeeEverything() const
{
    return getValue("spectatorsCanSeeEverything", "game").toBool();
}

bool GameSettings::getCreateGameAsSpectator() const
{
    return getValue("createGameAsSpectator", "game").toBool();
}

int GameSettings::getDefaultStartingLifeTotal() const
{
    return getValue("defaultStartingLifeTotal", "game", QString(), 20).toInt();
}

bool GameSettings::getShareDecklistsOnLoad() const
{
    return getValue("shareDecklistsOnLoad", "game").toBool();
}

bool GameSettings::getRememberGameSettings() const
{
    return getValue("rememberGameSettings", "game", QString(), true).toBool();
}

bool GameSettings::getLocalGameRememberSettings() const
{
    return getValue("rememberSettings", "localgameoptions").toBool();
}

int GameSettings::getLocalGameMaxPlayers() const
{
    return getValue("maxPlayers", "localgameoptions", QString(), 1).toInt();
}

int GameSettings::getLocalGameStartingLifeTotal() const
{
    return getValue("startingLifeTotal", "localgameoptions", QString(), 20).toInt();
}

void GameSettings::setGameDescription(const QString &_gameDescription)
{
    setValue(_gameDescription, "gameDescription", "game");
}

void GameSettings::setMaxPlayers(int _maxPlayers)
{
    setValue(_maxPlayers, "maxPlayers", "game");
}

void GameSettings::setGameTypes(const QString &_gameTypes)
{
    setValue(_gameTypes, "gameTypes", "game");
}

void GameSettings::setOnlyBuddies(bool _onlyBuddies)
{
    setValue(_onlyBuddies, "onlyBuddies", "game");
}

void GameSettings::setOnlyRegistered(bool _onlyRegistered)
{
    setValue(_onlyRegistered, "onlyRegistered", "game");
}

void GameSettings::setSpectatorsAllowed(bool _spectatorsAllowed)
{
    setValue(_spectatorsAllowed, "spectatorsAllowed", "game");
}

void GameSettings::setSpectatorsNeedPassword(bool _spectatorsNeedPassword)
{
    setValue(_spectatorsNeedPassword, "spectatorsNeedPassword", "game");
}

void GameSettings::setSpectatorsCanTalk(bool _spectatorsCanTalk)
{
    setValue(_spectatorsCanTalk, "spectatorsCanTalk", "game");
}

void GameSettings::setSpectatorsCanSeeEverything(bool _spectatorsCanSeeEverything)
{
    setValue(_spectatorsCanSeeEverything, "spectatorsCanSeeEverything", "game");
}

void GameSettings::setCreateGameAsSpectator(bool _createGameAsSpectator)
{
    setValue(_createGameAsSpectator, "createGameAsSpectator", "game");
}

void GameSettings::setDefaultStartingLifeTotal(int _defaultStartingLifeTotal)
{
    setValue(_defaultStartingLifeTotal, "defaultStartingLifeTotal", "game");
}

void GameSettings::setShareDecklistsOnLoad(bool _shareDecklistsOnLoad)
{
    setValue(_shareDecklistsOnLoad, "shareDecklistsOnLoad", "game");
}

void GameSettings::setRememberGameSettings(bool _rememberGameSettings)
{
    setValue(_rememberGameSettings, "rememberGameSettings", "game");
}

void GameSettings::setLocalGameRememberSettings(bool value)
{
    setValue(value, "rememberSettings", "localgameoptions");
}

void GameSettings::setLocalGameMaxPlayers(int value)
{
    setValue(value, "maxPlayers", "localgameoptions");
}

void GameSettings::setLocalGameStartingLifeTotal(int value)
{
    setValue(value, "startingLifeTotal", "localgameoptions");
}
