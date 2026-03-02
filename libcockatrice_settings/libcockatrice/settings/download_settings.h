/**
 * @file download_settings.h
 * @ingroup NetworkSettings
 * @brief TODO: Document this.
 */

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

    QStringList getAllURLs();
    void setDownloadUrls(const QStringList &downloadURLs);
    void resetToDefaultURLs();
};

#endif // COCKATRICE_DOWNLOADSETTINGS_H
