#include "game_filters_settings.h"

#include <QCryptographicHash>
#include <QTime>

GameFiltersSettings::GameFiltersSettings(const QString &settingPath, QObject *parent)
    : SettingsManager(settingPath + "gamefilters.ini", "filter_games", QString(), parent)
{
}

/**
 * The game type might contain special characters, so to use it in
 * QSettings we just hash it.
 */
static QString hashGameType(const QString &gameType)
{
    return QCryptographicHash::hash(gameType.toUtf8(), QCryptographicHash::Md5).toHex();
}

void GameFiltersSettings::setHideBuddiesOnlyGames(bool hide)
{
    setValue(hide, "hideBuddiesOnlyGames");
}

bool GameFiltersSettings::isHideBuddiesOnlyGames() const
{
    QVariant previous = getValue("hideBuddiesOnlyGames");
    return previous == QVariant() ? false : previous.toBool();
}

void GameFiltersSettings::setHideFullGames(bool hide)
{
    setValue(hide, "hideFullGames");
}

bool GameFiltersSettings::isHideFullGames() const
{
    QVariant previous = getValue("hideFullGames");
    return previous == QVariant() ? false : previous.toBool();
}

void GameFiltersSettings::setHideGamesThatStarted(bool hide)
{
    setValue(hide, "hideGamesThatStarted");
}

bool GameFiltersSettings::isHideGamesThatStarted() const
{
    QVariant previous = getValue("hideGamesThatStarted");
    return previous == QVariant() ? false : previous.toBool();
}

void GameFiltersSettings::setHidePasswordProtectedGames(bool hide)
{
    setValue(hide, "hidePasswordProtectedGames");
}

bool GameFiltersSettings::isHidePasswordProtectedGames() const
{
    QVariant previous = getValue("hidePasswordProtectedGames");
    return previous == QVariant() ? false : previous.toBool();
}

void GameFiltersSettings::setHideIgnoredUserGames(bool hide)
{
    setValue(hide, "hideIgnoredUserGames");
}

bool GameFiltersSettings::isHideIgnoredUserGames() const
{
    QVariant previous = getValue("hideIgnoredUserGames");
    return previous == QVariant() ? true : previous.toBool();
}

void GameFiltersSettings::setHideNotBuddyCreatedGames(bool hide)
{
    setValue(hide, "hideNotBuddyCreatedGames");
}

bool GameFiltersSettings::isHideNotBuddyCreatedGames() const
{
    QVariant previous = getValue("hideNotBuddyCreatedGames");
    return previous == QVariant() ? false : previous.toBool();
}

void GameFiltersSettings::setHideOpenDecklistGames(bool hide)
{
    setValue(hide, "hideOpenDecklistGames");
}

bool GameFiltersSettings::isHideOpenDecklistGames() const
{
    QVariant previous = getValue("hideOpenDecklistGames");
    return previous == QVariant() ? false : previous.toBool();
}

void GameFiltersSettings::setGameNameFilter(QString gameName)
{
    setValue(gameName, "gameNameFilter");
}

QString GameFiltersSettings::getGameNameFilter() const
{
    return getValue("gameNameFilter").toString();
}

void GameFiltersSettings::setHostNameFilters(QStringList hostName)
{
    setValue(hostName, "hostNameFilter");
}

QStringList GameFiltersSettings::getHostNameFilters() const
{
    return getValue("hostNameFilter").toStringList();
}

void GameFiltersSettings::setMinPlayers(int min)
{
    setValue(min, "minPlayers");
}

int GameFiltersSettings::getMinPlayers() const
{
    QVariant previous = getValue("minPlayers");
    return previous == QVariant() ? 1 : previous.toInt();
}

void GameFiltersSettings::setMaxPlayers(int max)
{
    setValue(max, "maxPlayers");
}

int GameFiltersSettings::getMaxPlayers() const
{
    QVariant previous = getValue("maxPlayers");
    return previous == QVariant() ? 99 : previous.toInt();
}

void GameFiltersSettings::setMaxGameAge(const QTime &maxGameAge)
{
    setValue(maxGameAge, "maxGameAgeTime");
}

QTime GameFiltersSettings::getMaxGameAge() const
{
    QVariant previous = getValue("maxGameAgeTime");
    return previous.toTime();
}

void GameFiltersSettings::setGameTypeEnabled(QString gametype, bool enabled)
{
    setValue(enabled, "gameType/" + hashGameType(gametype));
}

void GameFiltersSettings::setGameHashedTypeEnabled(QString gametypeHASHED, bool enabled)
{
    setValue(enabled, gametypeHASHED);
}

bool GameFiltersSettings::isGameTypeEnabled(QString gametype) const
{
    QVariant previous = getValue("gameType/" + hashGameType(gametype));
    return previous == QVariant() ? false : previous.toBool();
}

void GameFiltersSettings::setShowOnlyIfSpectatorsCanWatch(bool show)
{
    setValue(show, "showOnlyIfSpectatorsCanWatch");
}

bool GameFiltersSettings::isShowOnlyIfSpectatorsCanWatch() const
{
    QVariant previous = getValue("showOnlyIfSpectatorsCanWatch");
    return previous == QVariant() ? false : previous.toBool();
}

void GameFiltersSettings::setShowSpectatorPasswordProtected(bool show)
{
    setValue(show, "showSpectatorPasswordProtected");
}

bool GameFiltersSettings::isShowSpectatorPasswordProtected() const
{
    QVariant previous = getValue("showSpectatorPasswordProtected");
    return previous == QVariant() ? false : previous.toBool();
}

void GameFiltersSettings::setShowOnlyIfSpectatorsCanChat(bool show)
{
    setValue(show, "showOnlyIfSpectatorsCanChat");
}

bool GameFiltersSettings::isShowOnlyIfSpectatorsCanChat() const
{
    QVariant previous = getValue("showOnlyIfSpectatorsCanChat");
    return previous == QVariant() ? false : previous.toBool();
}

void GameFiltersSettings::setShowOnlyIfSpectatorsCanSeeHands(bool show)
{
    setValue(show, "showOnlyIfSpectatorsCanSeeHands");
}

bool GameFiltersSettings::isShowOnlyIfSpectatorsCanSeeHands() const
{
    QVariant previous = getValue("showOnlyIfSpectatorsCanSeeHands");
    return previous == QVariant() ? false : previous.toBool();
}