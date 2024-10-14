#ifndef GAMEFILTERSSETTINGS_H
#define GAMEFILTERSSETTINGS_H

#include "settings_manager.h"

class GameFiltersSettings : public SettingsManager
{
    Q_OBJECT
    friend class SettingsCache;

public:
    bool isShowBuddiesOnlyGames();
    bool isShowFullGames();
    bool isShowGamesThatStarted();
    bool isShowPasswordProtectedGames();
    bool isHideIgnoredUserGames();
    QString getGameNameFilter();
    QString getCreatorNameFilter();
    int getMinPlayers();
    int getMaxPlayers();
    QTime getMaxGameAge();
    bool isGameTypeEnabled(QString gametype);
    bool isShowOnlyIfSpectatorsCanWatch();
    bool isShowSpectatorPasswordProtected();
    bool isShowOnlyIfSpectatorsCanChat();
    bool isShowOnlyIfSpectatorsCanSeeHands();

    void setShowBuddiesOnlyGames(bool show);
    void setHideIgnoredUserGames(bool hide);
    void setShowFullGames(bool show);
    void setShowGamesThatStarted(bool show);
    void setShowPasswordProtectedGames(bool show);
    void setGameNameFilter(QString gameName);
    void setCreatorNameFilter(QString creatorName);
    void setMinPlayers(int min);
    void setMaxPlayers(int max);
    void setMaxGameAge(const QTime &maxGameAge);
    void setGameTypeEnabled(QString gametype, bool enabled);
    void setGameHashedTypeEnabled(QString gametypeHASHED, bool enabled);
    void setShowOnlyIfSpectatorsCanWatch(bool show);
    void setShowSpectatorPasswordProtected(bool show);
    void setShowOnlyIfSpectatorsCanChat(bool show);
    void setShowOnlyIfSpectatorsCanSeeHands(bool show);
signals:

public slots:

private:
    explicit GameFiltersSettings(QString settingPath, QObject *parent = nullptr);
    GameFiltersSettings(const GameFiltersSettings & /*other*/);

    QString hashGameType(const QString &gameType) const;
};

#endif // GAMEFILTERSSETTINGS_H
