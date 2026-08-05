#include "personal_settings.h"

PersonalSettings::PersonalSettings(const QString &settingPath, QObject *parent)
    : SettingsManager(settingPath + "personal.ini", "personal", QString(), parent)
{
}

QString PersonalSettings::getLang() const
{
    return getValue("lang", QString(), QString(), QString()).toString();
}

QString PersonalSettings::getClientID()
{
    return getValue("clientid", QString(), QString(), "notset").toString();
}

QString PersonalSettings::getClientVersion()
{
    return getValue("clientversion", QString(), QString(), "notset").toString();
}

int PersonalSettings::getKeepAlive() const
{
    return getValue("keepalive", QString(), QString(), 3).toInt();
}

int PersonalSettings::getTimeOut() const
{
    return getValue("timeout", QString(), QString(), 5).toInt();
}

bool PersonalSettings::getPicDownload() const
{
    return getValue("picturedownload", QString(), QString(), true).toBool();
}

bool PersonalSettings::getShowStatusBar() const
{
    return getValue("showStatusBar", QString(), QString(), false).toBool();
}

int PersonalSettings::getMaxFontSize() const
{
    return getValue("maxfontsize", "game", QString(), 12).toInt();
}

QString PersonalSettings::getHighlightWords() const
{
    return getValue("highlightWords", QString(), QString(), "").toString();
}

QString PersonalSettings::getHomeTabBackgroundSource() const
{
    return getValue("background", "home", QString(), "themed").toString();
}

int PersonalSettings::getHomeTabBackgroundShuffleFrequency() const
{
    return getValue("shuffleTimer", "home/background", QString(), 0).toInt();
}

bool PersonalSettings::getHomeTabDisplayCardName() const
{
    return getValue("displayCardName", "home/background", QString(), true).toBool();
}

bool PersonalSettings::getShowTipsOnStartup() const
{
    return getValue("showTips", "tipOfDay", QString(), true).toBool();
}

QList<int> PersonalSettings::getSeenTips() const
{
    QList<int> tips;
    auto tipValues = getValue("seenTips", "tipOfDay", QString()).toList();
    for (const auto &tipNumber : tipValues) {
        tips.append(tipNumber.toInt());
    }
    return tips;
}

bool PersonalSettings::getDownloadSpoilersStatus() const
{
    return getValue("downloadspoilers", QString(), QString(), false).toBool();
}

void PersonalSettings::setLang(const QString &_lang)
{
    setValue(_lang, "lang");
    emit langChanged();
}

void PersonalSettings::setClientID(const QString &_clientID)
{
    setValue(_clientID, "clientid");
}

void PersonalSettings::setClientVersion(const QString &_clientVersion)
{
    setValue(_clientVersion, "clientversion");
}

void PersonalSettings::setPicDownload(bool _picDownload)
{
    setValue(_picDownload, "picturedownload");
    emit picDownloadChanged();
}

void PersonalSettings::setShowStatusBar(bool value)
{
    setValue(value, "showStatusBar");
    emit showStatusBarChanged(value);
}

void PersonalSettings::setMaxFontSize(int _max)
{
    setValue(_max, "maxfontsize", "game");
}

QString PersonalSettings::getThemeName() const
{
    return getValue("themeName", QString(), QString()).toString();
}

void PersonalSettings::setThemeName(const QString &_themeName)
{
    setValue(_themeName, "themeName");
    emit themeNameChanged();
}

void PersonalSettings::setHighlightWords(const QString &_highlightWords)
{
    setValue(_highlightWords, "highlightWords");
}

void PersonalSettings::setHomeTabBackgroundSource(const QString &_backgroundSource)
{
    setValue(_backgroundSource, "background", "home");
    emit homeTabBackgroundSourceChanged();
}

void PersonalSettings::setHomeTabBackgroundShuffleFrequency(int _frequency)
{
    setValue(_frequency, "shuffleTimer", "home/background");
    emit homeTabBackgroundShuffleFrequencyChanged();
}

void PersonalSettings::setHomeTabDisplayCardName(bool _displayCardName)
{
    setValue(_displayCardName, "displayCardName", "home/background");
    emit homeTabDisplayCardNameChanged();
}

void PersonalSettings::setShowTipsOnStartup(bool _showTipsOnStartup)
{
    setValue(_showTipsOnStartup, "showTips", "tipOfDay");
}

void PersonalSettings::setSeenTips(const QList<int> &_seenTips)
{
    QList<QVariant> storedTipList;
    for (auto tipNumber : _seenTips) {
        storedTipList.append(tipNumber);
    }
    setValue(QVariant::fromValue(storedTipList), "seenTips", "tipOfDay");
}

void PersonalSettings::setDownloadSpoilerStatus(bool _spoilerStatus)
{
    setValue(_spoilerStatus, "downloadspoilers");
    emit downloadSpoilerStatusChanged();
}
