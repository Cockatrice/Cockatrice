#include "download_settings.h"

#include "settings_manager.h"

const QStringList DownloadSettings::DEFAULT_DOWNLOAD_URLS = {
    "https://api.scryfall.com/cards/!set:uuid!?format=image&face=!prop:side!",
    "https://api.scryfall.com/cards/multiverse/!set:muid!?format=image",
    "https://gatherer.wizards.com/Handlers/Image.ashx?multiverseid=!set:muid!&type=card",
    "https://gatherer.wizards.com/Handlers/Image.ashx?name=!name!&type=card"};

DownloadSettings::DownloadSettings(const QString &settingPath, QObject *parent = nullptr)
    : SettingsManager(settingPath + "downloads.ini", "downloads", QString(), parent)
{
}

void DownloadSettings::setDownloadUrls(const QStringList &downloadURLs)
{
    setValue(QVariant::fromValue(downloadURLs), "urls");
}

QStringList DownloadSettings::getAllURLs() const
{
    return getValue("urls").toStringList();
}

void DownloadSettings::resetToDefaultURLs()
{
    setValue(QVariant::fromValue(DEFAULT_DOWNLOAD_URLS), "urls");
}

bool DownloadSettings::getPicDownload() const
{
    return getValue("pictureDownload", QString(), QString(), true).toBool();
}

void DownloadSettings::setPicDownload(bool _picDownload)
{
    setValue(_picDownload, "pictureDownload");
    emit picDownloadChanged();
}

bool DownloadSettings::getDownloadSpoilersStatus() const
{
    return getValue("downloadSpoilers", QString(), QString(), false).toBool();
}

void DownloadSettings::setDownloadSpoilerStatus(bool _spoilerStatus)
{
    setValue(_spoilerStatus, "downloadSpoilers");
    emit downloadSpoilerStatusChanged();
}
