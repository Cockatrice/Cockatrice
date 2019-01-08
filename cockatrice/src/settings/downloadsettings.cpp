#include "downloadsettings.h"
#include "settingsmanager.h"

DownloadSettings::DownloadSettings(const QString &settingPath, QObject *parent = nullptr)
    : SettingsManager(settingPath + "downloads.ini", parent)
{
    downloadURLs = getValue("urls", "downloads").value<QList<QString>>();
}

void DownloadSettings::setDownloadUrlAt(int index, const QString &url)
{
    downloadURLs.insert(index, url);
    setValue(QVariant::fromValue(downloadURLs), "urls", "downloads");
}

QStringList DownloadSettings::getAllURLs()
{
    return downloadURLs;
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