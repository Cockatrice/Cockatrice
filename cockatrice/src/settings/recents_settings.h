/**
 * @file recents_settings.h
 * @ingroup DeckSettings
 * @brief TODO: Document this.
 */

#ifndef RECENTS_SETTINGS_H
#define RECENTS_SETTINGS_H

#include "settings_manager.h"

class RecentsSettings : public SettingsManager
{
    Q_OBJECT
    friend class SettingsCache;

    explicit RecentsSettings(const QString &settingPath, QObject *parent = nullptr);
    RecentsSettings(const RecentsSettings & /*other*/);

public:
    QStringList getRecentlyOpenedDeckPaths();
    void clearRecentlyOpenedDeckPaths();
    void updateRecentlyOpenedDeckPaths(const QString &deckPath);

    QString getLatestDeckDirPath();
    void setLatestDeckDirPath(const QString &dirPath);

signals:
    void recentlyOpenedDeckPathsChanged();
};

#endif // RECENTS_SETTINGS_H
