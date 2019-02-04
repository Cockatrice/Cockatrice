#include "downloadsettings.h"
#include "settingsmanager.h"

DownloadSettings::DownloadSettings(const QString &settingPath, QObject *parent = nullptr)
    : SettingsManager(settingPath + "downloads.ini", parent)
{
    downloadURLs = getValue("urls", "downloads").value<QStringList>();
}

void DownloadSettings::setDownloadUrlAt(int index, const QString &url)
{
    downloadURLs.insert(index, url);
    setValue(QVariant::fromValue(downloadURLs), "urls", "downloads");
}

/**
 * If reset or first run, this method contains the default URLs we will populate
 */
QStringList DownloadSettings::getAllURLs()
{
    // First run, these will be empty
    if (downloadURLs.count() == 0) {
        populateDefaultURLs();
    }

    return downloadURLs;
}

void DownloadSettings::populateDefaultURLs()
{
    downloadURLs.clear();
    downloadURLs.append("https://api.scryfall.com/cards/!set:uuid!?format=image&face=!prop:side!");
    downloadURLs.append("https://api.scryfall.com/cards/multiverse/!set:muid!?format=image");
    downloadURLs.append("http://gatherer.wizards.com/Handlers/Image.ashx?multiverseid=!set:muid!&type=card");
    downloadURLs.append("http://gatherer.wizards.com/Handlers/Image.ashx?name=!name!&type=card");
    setValue(QVariant::fromValue(downloadURLs), "urls", "downloads");
}

QString DownloadSettings::getDownloadUrlAt(int index)
{
    if (0 <= index && index < downloadURLs.size()) {
        return downloadURLs[index];
    }

    return "";
}

int DownloadSettings::getCount()
{
    return downloadURLs.size();
}

void DownloadSettings::clear()
{
    downloadURLs.clear();
}