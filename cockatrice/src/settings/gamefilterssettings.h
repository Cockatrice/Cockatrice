#ifndef GAMEFILTERSSETTINGS_H
#define GAMEFILTERSSETTINGS_H

#include "settingsmanager.h"

class GameFiltersSettings : public SettingsManager
{
    Q_OBJECT
    friend class SettingsCache;
public:
    bool isShowBuddiesOnlyGames();
    bool isUnavailableGamesVisible();
    bool isShowPasswordProtectedGames();
    QString getGameNameFilter();
    int getMinPlayers();
    int getMaxPlayers();
    bool isGameTypeEnabled(QString gametype);

    void setShowBuddiesOnlyGames(bool show);
    void setUnavailableGamesVisible(bool enabled);
    void setShowPasswordProtectedGames(bool show);
    void setGameNameFilter(QString gameName);
    void setMinPlayers(int min);
    void setMaxPlayers(int max);
    void setGameTypeEnabled(QString gametype, bool enabled);
    void setGameHashedTypeEnabled(QString gametypeHASHED, bool enabled);
signals:

public slots:

private:
    GameFiltersSettings(QString settingPath,QObject *parent = 0);
    GameFiltersSettings( const GameFiltersSettings& /*other*/ );
    GameFiltersSettings( GameFiltersSettings& /*other*/ );
    GameFiltersSettings( volatile const GameFiltersSettings& /*other*/ );
    GameFiltersSettings( volatile GameFiltersSettings& /*other*/ );

    QString hashGameType(const QString &gameType) const;
};

#endif // GAMEFILTERSSETTINGS_H
