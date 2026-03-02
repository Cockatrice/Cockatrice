/**
 * @file dlg_filter_games.h
 * @ingroup RoomDialogs
 * @brief TODO: Document this.
 */

#ifndef DLG_FILTER_GAMES_H
#define DLG_FILTER_GAMES_H

#include "../interface/widgets/server/games_model.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QMap>
#include <QSet>
#include <QTime>

class QCheckBox;
class QComboBox;
class QGroupBox;
class QLineEdit;
class QSpinBox;

class DlgFilterGames : public QDialog
{
    Q_OBJECT
private:
    QGroupBox *generalGroupBox;
    QCheckBox *hideBuddiesOnlyGames;
    QCheckBox *hideFullGames;
    QCheckBox *hideGamesThatStarted;
    QCheckBox *hidePasswordProtectedGames;
    QCheckBox *hideIgnoredUserGames;
    QCheckBox *hideNotBuddyCreatedGames;
    QCheckBox *hideOpenDecklistGames;
    QLineEdit *gameNameFilterEdit;
    QLineEdit *creatorNameFilterEdit;
    QMap<int, QCheckBox *> gameTypeFilterCheckBoxes;
    QSpinBox *maxPlayersFilterMinSpinBox;
    QSpinBox *maxPlayersFilterMaxSpinBox;
    QComboBox *maxGameAgeComboBox;

    QCheckBox *showOnlyIfSpectatorsCanWatch;
    QCheckBox *showSpectatorPasswordProtected;
    QCheckBox *showOnlyIfSpectatorsCanChat;
    QCheckBox *showOnlyIfSpectatorsCanSeeHands;

    const QMap<int, QString> &allGameTypes;
    const GamesProxyModel *gamesProxyModel;

private slots:
    void actOk();
    void toggleSpectatorCheckboxEnabledness(bool spectatorsEnabled);

public:
    DlgFilterGames(const QMap<int, QString> &_allGameTypes,
                   const GamesProxyModel *_gamesProxyModel,
                   QWidget *parent = nullptr);

    [[nodiscard]] bool getHideFullGames() const;
    [[nodiscard]] bool getHideGamesThatStarted() const;
    [[nodiscard]] bool getHidePasswordProtectedGames() const;
    void setShowPasswordProtectedGames(bool _passwordProtectedGamesHidden);
    [[nodiscard]] bool getHideBuddiesOnlyGames() const;
    void setHideBuddiesOnlyGames(bool _hideBuddiesOnlyGames);
    [[nodiscard]] bool getHideOpenDecklistGames() const;
    void setHideOpenDecklistGames(bool _hideOpenDecklistGames);
    [[nodiscard]] bool getHideIgnoredUserGames() const;
    void setHideIgnoredUserGames(bool _hideIgnoredUserGames);
    [[nodiscard]] bool getHideNotBuddyCreatedGames() const;
    [[nodiscard]] QString getGameNameFilter() const;
    void setGameNameFilter(const QString &_gameNameFilter);
    [[nodiscard]] QStringList getCreatorNameFilters() const;
    void setCreatorNameFilter(const QString &_creatorNameFilter);
    [[nodiscard]] QSet<int> getGameTypeFilter() const;
    void setGameTypeFilter(const QSet<int> &_gameTypeFilter);
    [[nodiscard]] int getMaxPlayersFilterMin() const;
    [[nodiscard]] int getMaxPlayersFilterMax() const;
    void setMaxPlayersFilter(int _maxPlayersFilterMin, int _maxPlayersFilterMax);
    [[nodiscard]] QTime getMaxGameAge() const;
    const QMap<QTime, QString> gameAgeMap;
    [[nodiscard]] bool getShowOnlyIfSpectatorsCanWatch() const;
    [[nodiscard]] bool getShowSpectatorPasswordProtected() const;
    [[nodiscard]] bool getShowOnlyIfSpectatorsCanChat() const;
    [[nodiscard]] bool getShowOnlyIfSpectatorsCanSeeHands() const;
};

#endif
