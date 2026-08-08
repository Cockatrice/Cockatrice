/**
 * @file download_settings.h
 * @ingroup NetworkSettings
 */
//! \todo Document this file.

#ifndef COCKATRICE_DOWNLOADSETTINGS_H
#define COCKATRICE_DOWNLOADSETTINGS_H

#include "settings_manager.h"

class DownloadSettings : public SettingsManager
{
    Q_OBJECT
    friend class SettingsCache;

    static const QStringList DEFAULT_DOWNLOAD_URLS;

public:
    explicit DownloadSettings(const QString &, QObject *);

    QStringList getAllURLs() const;
    void setDownloadUrls(const QStringList &downloadURLs);
    void resetToDefaultURLs();
    [[nodiscard]] bool getPicDownload() const;
    void setPicDownload(bool _picDownload);
    [[nodiscard]] bool getDownloadSpoilersStatus() const;
    void setDownloadSpoilerStatus(bool _spoilerStatus);

signals:
    void picDownloadChanged();
    void downloadSpoilerStatusChanged();
};

#endif // COCKATRICE_DOWNLOADSETTINGS_H
