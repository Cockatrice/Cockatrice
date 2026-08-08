#include "game_settings.h"

GameSettings::GameSettings(const QString &settingPath, QObject *parent)
    : SettingsManager(settingPath + "game.ini", QString(), QString(), parent)
{
}

QString GameSettings::getGameDescription() const
{
    return getValue("gamedescription", "game").toString();
}

int GameSettings::getMaxPlayers() const
{
    return getValue("maxplayers", "game", QString(), 2).toInt();
}

QString GameSettings::getGameTypes() const
{
    return getValue("gametypes", "game").toString();
}

bool GameSettings::getOnlyBuddies() const
{
    return getValue("onlybuddies", "game").toBool();
}

bool GameSettings::getOnlyRegistered() const
{
    return getValue("onlyregistered", "game").toBool();
}

bool GameSettings::getSpectatorsAllowed() const
{
    return getValue("spectatorsallowed", "game").toBool();
}

bool GameSettings::getSpectatorsNeedPassword() const
{
    return getValue("spectatorsneedpassword", "game").toBool();
}

bool GameSettings::getSpectatorsCanTalk() const
{
    return getValue("spectatorscantalk", "game").toBool();
}

bool GameSettings::getSpectatorsCanSeeEverything() const
{
    return getValue("spectatorscanseeeverything", "game").toBool();
}

bool GameSettings::getCreateGameAsSpectator() const
{
    return getValue("creategameasspectator", "game").toBool();
}

int GameSettings::getDefaultStartingLifeTotal() const
{
    return getValue("defaultstartinglifetotal", "game", QString(), 20).toInt();
}

bool GameSettings::getShareDecklistsOnLoad() const
{
    return getValue("sharedecklistsonload", "game").toBool();
}

bool GameSettings::getRememberGameSettings() const
{
    return getValue("remembergamesettings", "game", QString(), true).toBool();
}

bool GameSettings::getEnableCommandZone() const
{
    return getValue("enablecommandzone", "game").toBool();
}

bool GameSettings::getLocalGameRememberSettings() const
{
    return getValue("remembersettings", "localgameoptions").toBool();
}

int GameSettings::getLocalGameMaxPlayers() const
{
    return getValue("maxplayers", "localgameoptions", QString(), 1).toInt();
}

int GameSettings::getLocalGameStartingLifeTotal() const
{
    return getValue("startinglifetotal", "localgameoptions", QString(), 20).toInt();
}

bool GameSettings::getLocalGameEnableCommandZone() const
{
    return getValue("enablecommandzone", "localgameoptions").toBool();
}

void GameSettings::setGameDescription(const QString &_gameDescription)
{
    setValue(_gameDescription, "gamedescription", "game");
}

void GameSettings::setMaxPlayers(int _maxPlayers)
{
    setValue(_maxPlayers, "maxplayers", "game");
}

void GameSettings::setGameTypes(const QString &_gameTypes)
{
    setValue(_gameTypes, "gametypes", "game");
}

void GameSettings::setOnlyBuddies(bool _onlyBuddies)
{
    setValue(_onlyBuddies, "onlybuddies", "game");
}

void GameSettings::setOnlyRegistered(bool _onlyRegistered)
{
    setValue(_onlyRegistered, "onlyregistered", "game");
}

void GameSettings::setSpectatorsAllowed(bool _spectatorsAllowed)
{
    setValue(_spectatorsAllowed, "spectatorsallowed", "game");
}

void GameSettings::setSpectatorsNeedPassword(bool _spectatorsNeedPassword)
{
    setValue(_spectatorsNeedPassword, "spectatorsneedpassword", "game");
}

void GameSettings::setSpectatorsCanTalk(bool _spectatorsCanTalk)
{
    setValue(_spectatorsCanTalk, "spectatorscantalk", "game");
}

void GameSettings::setSpectatorsCanSeeEverything(bool _spectatorsCanSeeEverything)
{
    setValue(_spectatorsCanSeeEverything, "spectatorscanseeeverything", "game");
}

void GameSettings::setCreateGameAsSpectator(bool _createGameAsSpectator)
{
    setValue(_createGameAsSpectator, "creategameasspectator", "game");
}

void GameSettings::setDefaultStartingLifeTotal(int _defaultStartingLifeTotal)
{
    setValue(_defaultStartingLifeTotal, "defaultstartinglifetotal", "game");
}

void GameSettings::setShareDecklistsOnLoad(bool _shareDecklistsOnLoad)
{
    setValue(_shareDecklistsOnLoad, "sharedecklistsonload", "game");
}

void GameSettings::setRememberGameSettings(bool _rememberGameSettings)
{
    setValue(_rememberGameSettings, "remembergamesettings", "game");
}

void GameSettings::setEnableCommandZone(bool _enableCommandZone)
{
    setValue(_enableCommandZone, "enablecommandzone", "game");
}

void GameSettings::setLocalGameRememberSettings(bool value)
{
    setValue(value, "remembersettings", "localgameoptions");
}

void GameSettings::setLocalGameMaxPlayers(int value)
{
    setValue(value, "maxplayers", "localgameoptions");
}

void GameSettings::setLocalGameStartingLifeTotal(int value)
{
    setValue(value, "startinglifetotal", "localgameoptions");
}

void GameSettings::setLocalGameEnableCommandZone(bool value)
{
    setValue(value, "enablecommandzone", "localgameoptions");
}
