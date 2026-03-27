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
    bool isHideBuddiesOnlyGames() const;
    bool isHideFullGames() const;
    bool isHideGamesThatStarted() const;
    bool isHidePasswordProtectedGames() const;
    bool isHideIgnoredUserGames() const;
    bool isHideNotBuddyCreatedGames() const;
    bool isHideOpenDecklistGames() const;
    QString getGameNameFilter() const;
    QStringList getCreatorNameFilters() const;
    int getMinPlayers() const;
    int getMaxPlayers() const;
    QTime getMaxGameAge() const;
    bool isGameTypeEnabled(QString gametype) const;
    bool isShowOnlyIfSpectatorsCanWatch() const;
    bool isShowSpectatorPasswordProtected() const;
    bool isShowOnlyIfSpectatorsCanChat() const;
    bool isShowOnlyIfSpectatorsCanSeeHands() const;

    void setHideBuddiesOnlyGames(bool hide);
    void setHideIgnoredUserGames(bool hide);
    void setHideOpenDecklistGames(bool hide);
    void setHideFullGames(bool hide);
    void setHideGamesThatStarted(bool hide);
    void setHidePasswordProtectedGames(bool hide);
    void setHideNotBuddyCreatedGames(bool hide);
    void setGameNameFilter(QString gameName);
    void setCreatorNameFilters(QStringList creatorName);
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

    [[nodiscard]] static QString hashGameType(const QString &gameType);
};

#endif // GAMEFILTERSSETTINGS_H
