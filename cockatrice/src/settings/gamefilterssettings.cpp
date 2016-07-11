#include "gamefilterssettings.h"
#include <QCryptographicHash>

GameFiltersSettings::GameFiltersSettings(QString settingPath, QObject *parent)
    : SettingsManager(settingPath+"gamefilters.ini", parent)
{
}

/*
 * The game type might contain special characters, so to use it in
 * QSettings we just hash it.
 */
QString GameFiltersSettings::hashGameType(const QString &gameType) const
{
    return QCryptographicHash::hash(gameType.toUtf8(), QCryptographicHash::Md5).toHex();
}

void GameFiltersSettings::setShowBuddiesOnlyGames(bool show)
{
    setValue(show, "show_buddies_only_games", "filter_games");
}

bool GameFiltersSettings::isShowBuddiesOnlyGames()
{
    QVariant previous = getValue("show_buddies_only_games", "filter_games");
    return previous == QVariant() ? true : previous.toBool();
}

void GameFiltersSettings::setUnavailableGamesVisible(bool enabled)
{
    setValue(enabled, "unavailable_games_visible","filter_games");
}

bool GameFiltersSettings::isUnavailableGamesVisible()
{
    QVariant previous = getValue("unavailable_games_visible","filter_games");
    return previous == QVariant() ? false : previous.toBool();
}

void GameFiltersSettings::setShowPasswordProtectedGames(bool show)
{
    setValue(show, "show_password_protected_games","filter_games");
}

bool GameFiltersSettings::isShowPasswordProtectedGames()
{
    QVariant previous = getValue("show_password_protected_games","filter_games");
    return previous == QVariant() ? true : previous.toBool();
}

void GameFiltersSettings::setGameNameFilter(QString gameName)
{
    setValue(gameName, "game_name_filter","filter_games");
}

QString GameFiltersSettings::getGameNameFilter()
{
    return getValue("game_name_filter","filter_games").toString();
}

void GameFiltersSettings::setMinPlayers(int min)
{
    setValue(min, "min_players","filter_games");
}

int GameFiltersSettings::getMinPlayers()
{
    QVariant previous = getValue("min_players","filter_games");
    return previous == QVariant() ? 1 : previous.toInt();
}

void GameFiltersSettings::setMaxPlayers(int max)
{
    setValue(max, "max_players","filter_games");
}

int GameFiltersSettings::getMaxPlayers()
{
    QVariant previous = getValue("max_players","filter_games");
    return previous == QVariant() ? 99 : previous.toInt();
}

void GameFiltersSettings::setGameTypeEnabled(QString gametype, bool enabled)
{
    setValue(enabled, "game_type/"+hashGameType(gametype),"filter_games");
}

void GameFiltersSettings::setGameHashedTypeEnabled(QString gametypeHASHED, bool enabled)
{
    setValue(enabled, gametypeHASHED,"filter_games");
}

bool GameFiltersSettings::isGameTypeEnabled(QString gametype)
{
    QVariant previous = getValue("game_type/"+hashGameType(gametype),"filter_games");
    return previous == QVariant() ? false : previous.toBool();
}


