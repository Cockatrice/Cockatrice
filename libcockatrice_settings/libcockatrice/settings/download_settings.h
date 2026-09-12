/**
 * @file download_settings.h
 * @ingroup NetworkSettings
 */
//! \todo Document this file.

#ifndef COCKATRICE_DOWNLOADSETTINGS_H
#define COCKATRICE_DOWNLOADSETTINGS_H

#include "settings_manager.h"

#include <QHash>

class DownloadSettings : public SettingsManager
{
    Q_OBJECT
    friend class SettingsCache;

    static const QStringList DEFAULT_DOWNLOAD_URLS;
    static const QHash<QString, int> DEVELOPER_HOST_CAPS;

public:
    /** @brief Per-host request allowance (requests/second) when no developer cap applies. */
    static constexpr int DEFAULT_HOST_REQUEST_LIMIT = 10;
    /** @brief Floor for any per-host request allowance. */
    static constexpr int MIN_HOST_REQUEST_LIMIT = 1;
    /** @brief Developer cap marking a host as never throttled per host (pacing still applies). */
    static constexpr int UNLIMITED_HOST_QUOTA = -1;

    /**
     * @brief Developer-set per-host allowance ceilings (requests/second), keyed by host.
     *
     * Hosts not present default to DEFAULT_HOST_REQUEST_LIMIT. An entry of
     * UNLIMITED_HOST_QUOTA marks a host that users may still lower, but that is never
     * throttled per host by default. Users can never raise a host's allowance above its
     * developer cap.
     */
    static const QHash<QString, int> &getDeveloperHostCaps();

    explicit DownloadSettings(const QString &, QObject *);

    QStringList getAllURLs() const;
    void setDownloadUrls(const QStringList &downloadURLs);
    void resetToDefaultURLs();
    [[nodiscard]] bool getPicDownload() const;
    void setPicDownload(bool _picDownload);
    [[nodiscard]] bool getDownloadSpoilersStatus() const;
    void setDownloadSpoilerStatus(bool _spoilerStatus);

    /** @brief User-set per-host request allowances (requests/second). Missing hosts use the developer default. */
    QHash<QString, int> getHostRequestLimits() const;
    void setHostRequestLimits(const QHash<QString, int> &hostRequestLimits);

    /**
     * @brief Clamps the user's requested allowance for a host against its developer cap.
     * @param host The host to clamp for
     * @param requested The user-requested allowance in requests/second
     * @return The effective allowance. Users may lower a host's allowance but never raise it
     *         above the developer cap; hosts with UNLIMITED_HOST_QUOTA have no upper bound.
     */
    [[nodiscard]] int clampHostRequestLimit(const QString &host, int requested) const;

signals:
    void picDownloadChanged();
    void downloadSpoilerStatusChanged();
    void hostRequestLimitsChanged();
};

#endif // COCKATRICE_DOWNLOADSETTINGS_H
