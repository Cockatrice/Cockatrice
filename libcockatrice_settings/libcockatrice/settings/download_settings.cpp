#include "download_settings.h"

#include "settings_manager.h"

const QStringList DownloadSettings::DEFAULT_DOWNLOAD_URLS = {
    "https://cards.scryfall.io/large/!prop:side!/!set:uuid_substr_0_1!/!set:uuid_substr_1_1!/!set:uuid!.jpg",
    "https://api.scryfall.com/cards/!set:uuid!?format=image&face=!prop:side!",
    "https://api.scryfall.com/cards/multiverse/!set:muid!?format=image",
    "https://gatherer.wizards.com/Handlers/Image.ashx?multiverseid=!set:muid!&type=card",
    "https://gatherer.wizards.com/Handlers/Image.ashx?name=!name!&type=card"};

// Developer-set ceilings for the per-host request allowance. Users may lower a host's
// allowance via the download settings, but can never raise it above these values. Hosts
// not listed default to DEFAULT_HOST_REQUEST_LIMIT. A cap of UNLIMITED_HOST_QUOTA marks a
// host that is never throttled per host and skips the dispatch pacing (429 backoff still applies).
const QHash<QString, int> DownloadSettings::DEVELOPER_HOST_CAPS = {
    // The Scryfall API enforces 10 requests/second; stay one under so a burst can't trip 429s.
    {"api.scryfall.com", 9},
    // The Scryfall image CDN has no documented per-client rate limit.
    {"cards.scryfall.io", UNLIMITED_HOST_QUOTA},
};

const QHash<QString, int> &DownloadSettings::getDeveloperHostCaps()
{
    return DEVELOPER_HOST_CAPS;
}

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

QHash<QString, int> DownloadSettings::getHostRequestLimits() const
{
    const QVariantMap stored = getValue("hostRequestLimits").toMap();
    QHash<QString, int> hostRequestLimits;
    for (auto it = stored.cbegin(); it != stored.cend(); ++it) {
        hostRequestLimits.insert(it.key(), it.value().toInt());
    }
    return hostRequestLimits;
}

void DownloadSettings::setHostRequestLimits(const QHash<QString, int> &hostRequestLimits)
{
    QVariantMap stored;
    for (auto it = hostRequestLimits.cbegin(); it != hostRequestLimits.cend(); ++it) {
        stored.insert(it.key(), it.value());
    }
    setValue(stored, "hostRequestLimits");
    emit hostRequestLimitsChanged();
}

int DownloadSettings::clampHostRequestLimit(const QString &host, int requested) const
{
    const int devCap = DEVELOPER_HOST_CAPS.value(host, DEFAULT_HOST_REQUEST_LIMIT);
    if (devCap == UNLIMITED_HOST_QUOTA) {
        return qMax(MIN_HOST_REQUEST_LIMIT, requested);
    }
    return qBound(MIN_HOST_REQUEST_LIMIT, requested, devCap);
}
