#include "paths_settings.h"

#include <QDir>
#include <QFile>

PathsSettings::PathsSettings(const QString &settingPath, QObject *parent)
    : SettingsManager(settingPath + "paths.ini", "paths", QString(), parent)
{
}

QString PathsSettings::getDeckPath() const
{
    return getValue("decks").toString();
}

QString PathsSettings::getFiltersPath() const
{
    return getValue("filters").toString();
}

QString PathsSettings::getReplaysPath() const
{
    return getValue("replays").toString();
}

QString PathsSettings::getPicsPath() const
{
    return getValue("pics").toString();
}

QString PathsSettings::getCustomPicsPath() const
{
    return getValue("customPics").toString();
}

QString PathsSettings::getThemesPath() const
{
    return getValue("themes").toString();
}

QString PathsSettings::getCardDatabasePath() const
{
    return getValue("cardDatabase").toString();
}

QString PathsSettings::getCustomCardDatabasePath() const
{
    return getValue("customSets").toString();
}

QString PathsSettings::getTokenDatabasePath() const
{
    return getValue("tokenDatabase").toString();
}

QString PathsSettings::getSpoilerCardDatabasePath() const
{
    return getValue("spoilerDatabase").toString();
}

QString PathsSettings::getRedirectCachePath() const
{
    return getValue("redirects").toString();
}

void PathsSettings::setDeckPath(const QString &_deckPath)
{
    setValue(_deckPath, "decks");
}

void PathsSettings::setFiltersPath(const QString &_filtersPath)
{
    setValue(_filtersPath, "filters");
}

void PathsSettings::setReplaysPath(const QString &_replaysPath)
{
    setValue(_replaysPath, "replays");
}

void PathsSettings::setPicsPath(const QString &_picsPath)
{
    setValue(_picsPath, "pics");
    emit picsPathChanged();
}

void PathsSettings::setCustomPicsPath(const QString &_customPicsPath)
{
    setValue(_customPicsPath, "customPics");
}

void PathsSettings::setThemesPath(const QString &_themesPath)
{
    setValue(_themesPath, "themes");
    emit themeChanged();
}

void PathsSettings::setCardDatabasePath(const QString &_cardDatabasePath)
{
    setValue(_cardDatabasePath, "cardDatabase");
    emit cardDatabasePathChanged();
}

void PathsSettings::setCustomCardDatabasePath(const QString &_customCardDatabasePath)
{
    setValue(_customCardDatabasePath, "customSets");
    emit cardDatabasePathChanged();
}

void PathsSettings::setTokenDatabasePath(const QString &_tokenDatabasePath)
{
    setValue(_tokenDatabasePath, "tokenDatabase");
    emit cardDatabasePathChanged();
}

void PathsSettings::setSpoilerDatabasePath(const QString &_spoilerDatabasePath)
{
    setValue(_spoilerDatabasePath, "spoilerDatabase");
    emit cardDatabasePathChanged();
}
