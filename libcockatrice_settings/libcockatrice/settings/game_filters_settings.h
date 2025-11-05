/**
 * @file game_filters_settings.h
 * @ingroup Lobby
 * @ingroup GameSettings
 * @brief TODO: Document this.
 */

#ifndef GAMEFILTERSSETTINGS_H
#define GAMEFILTERSSETTINGS_H

#include "settings_manager.h"

class GameFiltersSettings : public SettingsManager
{
    Q_OBJECT
    friend class SettingsCache;

public:
    bool isHideBuddiesOnlyGames();
    bool isHideFullGames();
    bool isHideGamesThatStarted();
    bool isHidePasswordProtectedGames();
    bool isHideIgnoredUserGames();
    bool isHideNotBuddyCreatedGames();
    bool isHideOpenDecklistGames();
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

    void setHideBuddiesOnlyGames(bool hide);
    void setHideIgnoredUserGames(bool hide);
    void setHideOpenDecklistGames(bool hide);
    void setHideFullGames(bool hide);
    void setHideGamesThatStarted(bool hide);
    void setHidePasswordProtectedGames(bool hide);
    void setHideNotBuddyCreatedGames(bool hide);
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

private:
    explicit GameFiltersSettings(const QString &settingPath, QObject *parent = nullptr);
    GameFiltersSettings(const GameFiltersSettings & /*other*/);

    QString hashGameType(const QString &gameType) const;
};

#endif // GAMEFILTERSSETTINGS_H
