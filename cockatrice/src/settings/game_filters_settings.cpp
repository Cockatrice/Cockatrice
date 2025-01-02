#include "game_filters_settings.h"

#include <QCryptographicHash>
#include <QTime>

GameFiltersSettings::GameFiltersSettings(const QString &settingPath, QObject *parent)
    : SettingsManager(settingPath + "gamefilters.ini", parent)
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

void GameFiltersSettings::setShowFullGames(bool show)
{
    setValue(show, "show_full_games", "filter_games");
}

bool GameFiltersSettings::isShowFullGames()
{
    QVariant previous = getValue("show_full_games", "filter_games");
    return previous == QVariant() ? false : previous.toBool();
}

void GameFiltersSettings::setShowGamesThatStarted(bool show)
{
    setValue(show, "show_games_that_started", "filter_games");
}

bool GameFiltersSettings::isShowGamesThatStarted()
{
    QVariant previous = getValue("show_games_that_started", "filter_games");
    return previous == QVariant() ? false : previous.toBool();
}

void GameFiltersSettings::setShowPasswordProtectedGames(bool show)
{
    setValue(show, "show_password_protected_games", "filter_games");
}

bool GameFiltersSettings::isShowPasswordProtectedGames()
{
    QVariant previous = getValue("show_password_protected_games", "filter_games");
    return previous == QVariant() ? true : previous.toBool();
}

void GameFiltersSettings::setHideIgnoredUserGames(bool hide)
{
    setValue(hide, "hide_ignored_user_games", "filter_games");
}

bool GameFiltersSettings::isHideIgnoredUserGames()
{
    QVariant previous = getValue("hide_ignored_user_games", "filter_games");
    return previous == QVariant() ? false : previous.toBool();
}

void GameFiltersSettings::setGameNameFilter(QString gameName)
{
    setValue(gameName, "game_name_filter", "filter_games");
}

QString GameFiltersSettings::getGameNameFilter()
{
    return getValue("game_name_filter", "filter_games").toString();
}

void GameFiltersSettings::setCreatorNameFilter(QString creatorName)
{
    setValue(creatorName, "creator_name_filter", "filter_games");
}

QString GameFiltersSettings::getCreatorNameFilter()
{
    return getValue("creator_name_filter", "filter_games").toString();
}

void GameFiltersSettings::setMinPlayers(int min)
{
    setValue(min, "min_players", "filter_games");
}

int GameFiltersSettings::getMinPlayers()
{
    QVariant previous = getValue("min_players", "filter_games");
    return previous == QVariant() ? 1 : previous.toInt();
}

void GameFiltersSettings::setMaxPlayers(int max)
{
    setValue(max, "max_players", "filter_games");
}

int GameFiltersSettings::getMaxPlayers()
{
    QVariant previous = getValue("max_players", "filter_games");
    return previous == QVariant() ? 99 : previous.toInt();
}

void GameFiltersSettings::setMaxGameAge(const QTime &maxGameAge)
{
    setValue(maxGameAge, "max_game_age_time", "filter_games");
}

QTime GameFiltersSettings::getMaxGameAge()
{
    QVariant previous = getValue("max_game_age_time", "filter_games");
    return previous.toTime();
}

void GameFiltersSettings::setGameTypeEnabled(QString gametype, bool enabled)
{
    setValue(enabled, "game_type/" + hashGameType(gametype), "filter_games");
}

void GameFiltersSettings::setGameHashedTypeEnabled(QString gametypeHASHED, bool enabled)
{
    setValue(enabled, gametypeHASHED, "filter_games");
}

bool GameFiltersSettings::isGameTypeEnabled(QString gametype)
{
    QVariant previous = getValue("game_type/" + hashGameType(gametype), "filter_games");
    return previous == QVariant() ? false : previous.toBool();
}

void GameFiltersSettings::setShowOnlyIfSpectatorsCanWatch(bool show)
{
    setValue(show, "show_only_if_spectators_can_watch", "filter_games");
}

bool GameFiltersSettings::isShowOnlyIfSpectatorsCanWatch()
{
    QVariant previous = getValue("show_only_if_spectators_can_watch", "filter_games");
    return previous == QVariant() ? false : previous.toBool();
}

void GameFiltersSettings::setShowSpectatorPasswordProtected(bool show)
{
    setValue(show, "show_spectator_password_protected", "filter_games");
}

bool GameFiltersSettings::isShowSpectatorPasswordProtected()
{
    QVariant previous = getValue("show_spectator_password_protected", "filter_games");
    return previous == QVariant() ? true : previous.toBool();
}

void GameFiltersSettings::setShowOnlyIfSpectatorsCanChat(bool show)
{
    setValue(show, "show_only_if_spectators_can_chat", "filter_games");
}

bool GameFiltersSettings::isShowOnlyIfSpectatorsCanChat()
{
    QVariant previous = getValue("show_only_if_spectators_can_chat", "filter_games");
    return previous == QVariant() ? true : previous.toBool();
}

void GameFiltersSettings::setShowOnlyIfSpectatorsCanSeeHands(bool show)
{
    setValue(show, "show_only_if_spectators_can_see_hands", "filter_games");
}

bool GameFiltersSettings::isShowOnlyIfSpectatorsCanSeeHands()
{
    QVariant previous = getValue("show_only_if_spectators_can_see_hands", "filter_games");
    return previous == QVariant() ? true : previous.toBool();
}