#include "download_settings.h"

#include "settings_manager.h"

DownloadSettings::DownloadSettings(const QString &settingPath, QObject *parent = nullptr)
    : SettingsManager(settingPath + "downloads.ini", parent)
{
}

void DownloadSettings::setDownloadUrls(const QStringList &downloadURLs)
{
    setValue(QVariant::fromValue(downloadURLs), "urls", "downloads");
}

/**
 * If reset or first run, this method contains the default URLs we will populate
 */
QStringList DownloadSettings::getAllURLs()
{
    auto downloadURLs = getValue("urls", "downloads").toStringList();

    // First run, these will be empty
    if (downloadURLs.count() == 0) {
        resetToDefaultURLs();
    }

    return downloadURLs;
}

void DownloadSettings::resetToDefaultURLs()
{
    auto downloadURLs = QStringList();
    downloadURLs.append("https://api.scryfall.com/cards/!set:uuid!?format=image&face=!prop:side!");
    downloadURLs.append("https://api.scryfall.com/cards/multiverse/!set:muid!?format=image");
    downloadURLs.append("https://gatherer.wizards.com/Handlers/Image.ashx?multiverseid=!set:muid!&type=card");
    downloadURLs.append("https://gatherer.wizards.com/Handlers/Image.ashx?name=!name!&type=card");
    setValue(QVariant::fromValue(downloadURLs), "urls", "downloads");
}
