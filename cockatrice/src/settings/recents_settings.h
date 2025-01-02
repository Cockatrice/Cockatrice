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

signals:
    void recentlyOpenedDeckPathsChanged();
};

#endif // RECENTS_SETTINGS_H
