#ifndef DECK_SYNC_SETTINGS_H
#define DECK_SYNC_SETTINGS_H

#include "settings_manager.h"

#include <QString>
#include <qtmetamacros.h>

class QObject;

enum DeckSyncConflictDefault
{
    DeckSyncConflictAsk = 0,    ///< Ask every time a deck changed on both sides.
    DeckSyncConflictKeepMine,   ///< Silently keep the local deck.
    DeckSyncConflictKeepServer, ///< Silently keep the server deck.
    DeckSyncConflictKeepNewer,  ///< Silently keep whichever side changed last.
};

class DeckSyncSettings : public SettingsManager
{
    Q_OBJECT
    friend class SettingsCache;

public:
    explicit DeckSyncSettings(const QString &settingPath, QObject *parent = nullptr);

    [[nodiscard]] int getConflictDefault() const;
    void setConflictDefault(int _conflictDefault);
    [[nodiscard]] bool getAutoSync() const;
    void setAutoSync(bool _autoSync);
    [[nodiscard]] int getSyncIntervalMinutes() const;
    void setSyncIntervalMinutes(int _syncIntervalMinutes);

signals:
    void conflictDefaultChanged(int conflictDefault);
    void autoSyncChanged(bool autoSync);
    void syncIntervalMinutesChanged(int syncIntervalMinutes);

private:
    DeckSyncSettings(const DeckSyncSettings & /*other*/);
};

#endif // DECK_SYNC_SETTINGS_H
