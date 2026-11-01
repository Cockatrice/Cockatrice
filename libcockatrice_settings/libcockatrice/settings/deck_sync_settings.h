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
    [[nodiscard]] int getConflictDefault() const;
    void setConflictDefault(int _conflictDefault);

signals:
    void conflictDefaultChanged(int conflictDefault);

public:
    explicit DeckSyncSettings(const QString &settingPath, QObject *parent = nullptr);

private:
    DeckSyncSettings(const DeckSyncSettings & /*other*/);
};

#endif // DECK_SYNC_SETTINGS_H
