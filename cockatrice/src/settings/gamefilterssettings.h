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
    bool isHideIgnoredUserGames();
    QString getGameNameFilter();
    int getMinPlayers();
    int getMaxPlayers();
    int getMaxGameAgeEnum();
    bool isGameTypeEnabled(QString gametype);

    void setShowBuddiesOnlyGames(bool show);
    void setHideIgnoredUserGames(bool hide);
    void setUnavailableGamesVisible(bool enabled);
    void setShowPasswordProtectedGames(bool show);
    void setGameNameFilter(QString gameName);
    void setMinPlayers(int min);
    void setMaxPlayers(int max);
    void setMaxGameAgeAsInt(int maxGameAgeEnum);
    void setGameTypeEnabled(QString gametype, bool enabled);
    void setGameHashedTypeEnabled(QString gametypeHASHED, bool enabled);
signals:

public slots:

private:
    explicit GameFiltersSettings(QString settingPath, QObject *parent = nullptr);
    GameFiltersSettings(const GameFiltersSettings & /*other*/);

    QString hashGameType(const QString &gameType) const;
};

#endif // GAMEFILTERSSETTINGS_H
