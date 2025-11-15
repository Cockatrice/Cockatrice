#include "game_filters_settings.h"

#include <QCryptographicHash>
#include <QTime>

GameFiltersSettings::GameFiltersSettings(const QString &settingPath, QObject *parent)
    : SettingsManager(settingPath + "gamefilters.ini", "filter_games", QString(), parent)
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

void GameFiltersSettings::setHideBuddiesOnlyGames(bool hide)
{
    setValue(hide, "hide_buddies_only_games");
}

bool GameFiltersSettings::isHideBuddiesOnlyGames()
{
    QVariant previous = getValue("hide_buddies_only_games");
    return previous == QVariant() ? false : previous.toBool();
}

void GameFiltersSettings::setHideFullGames(bool hide)
{
    setValue(hide, "hide_full_games");
}

bool GameFiltersSettings::isHideFullGames()
{
    QVariant previous = getValue("hide_full_games");
    return previous == QVariant() ? false : previous.toBool();
}

void GameFiltersSettings::setHideGamesThatStarted(bool hide)
{
    setValue(hide, "hide_games_that_started");
}

bool GameFiltersSettings::isHideGamesThatStarted()
{
    QVariant previous = getValue("hide_games_that_started");
    return previous == QVariant() ? false : previous.toBool();
}

void GameFiltersSettings::setHidePasswordProtectedGames(bool hide)
{
    setValue(hide, "hide_password_protected_games");
}

bool GameFiltersSettings::isHidePasswordProtectedGames()
{
    QVariant previous = getValue("hide_password_protected_games");
    return previous == QVariant() ? false : previous.toBool();
}

void GameFiltersSettings::setHideIgnoredUserGames(bool hide)
{
    setValue(hide, "hide_ignored_user_games");
}

bool GameFiltersSettings::isHideIgnoredUserGames()
{
    QVariant previous = getValue("hide_ignored_user_games");
    return previous == QVariant() ? true : previous.toBool();
}

void GameFiltersSettings::setHideNotBuddyCreatedGames(bool hide)
{
    setValue(hide, "hide_not_buddy_created_games");
}

bool GameFiltersSettings::isHideNotBuddyCreatedGames()
{
    QVariant previous = getValue("hide_not_buddy_created_games");
    return previous == QVariant() ? false : previous.toBool();
}

void GameFiltersSettings::setHideOpenDecklistGames(bool hide)
{
    setValue(hide, "hide_open_decklist_games");
}

bool GameFiltersSettings::isHideOpenDecklistGames()
{
    QVariant previous = getValue("hide_open_decklist_games");
    return previous == QVariant() ? false : previous.toBool();
}

void GameFiltersSettings::setGameNameFilter(QString gameName)
{
    setValue(gameName, "game_name_filter");
}

QString GameFiltersSettings::getGameNameFilter()
{
    return getValue("game_name_filter").toString();
}

void GameFiltersSettings::setCreatorNameFilters(QStringList creatorName)
{
    setValue(creatorName, "creator_name_filter");
}

QStringList GameFiltersSettings::getCreatorNameFilters()
{
    return getValue("creator_name_filter").toStringList();
}

void GameFiltersSettings::setMinPlayers(int min)
{
    setValue(min, "min_players");
}

int GameFiltersSettings::getMinPlayers()
{
    QVariant previous = getValue("min_players");
    return previous == QVariant() ? 1 : previous.toInt();
}

void GameFiltersSettings::setMaxPlayers(int max)
{
    setValue(max, "max_players");
}

int GameFiltersSettings::getMaxPlayers()
{
    QVariant previous = getValue("max_players");
    return previous == QVariant() ? 99 : previous.toInt();
}

void GameFiltersSettings::setMaxGameAge(const QTime &maxGameAge)
{
    setValue(maxGameAge, "max_game_age_time");
}

QTime GameFiltersSettings::getMaxGameAge()
{
    QVariant previous = getValue("max_game_age_time");
    return previous.toTime();
}

void GameFiltersSettings::setGameTypeEnabled(QString gametype, bool enabled)
{
    setValue(enabled, "game_type/" + hashGameType(gametype));
}

void GameFiltersSettings::setGameHashedTypeEnabled(QString gametypeHASHED, bool enabled)
{
    setValue(enabled, gametypeHASHED);
}

bool GameFiltersSettings::isGameTypeEnabled(QString gametype)
{
    QVariant previous = getValue("game_type/" + hashGameType(gametype));
    return previous == QVariant() ? false : previous.toBool();
}

void GameFiltersSettings::setShowOnlyIfSpectatorsCanWatch(bool show)
{
    setValue(show, "show_only_if_spectators_can_watch");
}

bool GameFiltersSettings::isShowOnlyIfSpectatorsCanWatch()
{
    QVariant previous = getValue("show_only_if_spectators_can_watch");
    return previous == QVariant() ? false : previous.toBool();
}

void GameFiltersSettings::setShowSpectatorPasswordProtected(bool show)
{
    setValue(show, "show_spectator_password_protected");
}

bool GameFiltersSettings::isShowSpectatorPasswordProtected()
{
    QVariant previous = getValue("show_spectator_password_protected");
    return previous == QVariant() ? false : previous.toBool();
}

void GameFiltersSettings::setShowOnlyIfSpectatorsCanChat(bool show)
{
    setValue(show, "show_only_if_spectators_can_chat");
}

bool GameFiltersSettings::isShowOnlyIfSpectatorsCanChat()
{
    QVariant previous = getValue("show_only_if_spectators_can_chat");
    return previous == QVariant() ? false : previous.toBool();
}

void GameFiltersSettings::setShowOnlyIfSpectatorsCanSeeHands(bool show)
{
    setValue(show, "show_only_if_spectators_can_see_hands");
}

bool GameFiltersSettings::isShowOnlyIfSpectatorsCanSeeHands()
{
    QVariant previous = getValue("show_only_if_spectators_can_see_hands");
    return previous == QVariant() ? false : previous.toBool();
}