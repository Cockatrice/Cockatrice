/**
 * @file dlg_filter_games.h
 * @ingroup RoomDialogs
 */
//! \todo Document this file.

#ifndef DLG_FILTER_GAMES_H
#define DLG_FILTER_GAMES_H

#include "../server/game_filter_configs.h"

#include <QDialog>
#include <QMap>
#include <QSet>
#include <QString>
#include <QStringList>
#include <QTime>
#include <qtmetamacros.h>

class QCheckBox;
class QComboBox;
class QGroupBox;
class QLineEdit;
class QSpinBox;
class GamesProxyModel;
class QWidget;

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
    QLineEdit *hostNameFilterEdit;
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
    const QMap<QTime, QString> gameAgeMap;

    [[nodiscard]] QStringList getHostNameFilters() const;
    [[nodiscard]] QSet<int> getGameTypeFilter() const;
    [[nodiscard]] QTime getMaxGameAge() const;
    [[nodiscard]] bool getShowSpectatorPasswordProtected() const;
    [[nodiscard]] bool getShowOnlyIfSpectatorsCanChat() const;
    [[nodiscard]] bool getShowOnlyIfSpectatorsCanSeeHands() const;

private slots:
    void actOk();
    void toggleSpectatorCheckboxEnabledness(bool spectatorsEnabled);

public:
    DlgFilterGames(const QMap<int, QString> &_allGameTypes,
                   const GamesProxyModel *_gamesProxyModel,
                   QWidget *parent = nullptr);

    [[nodiscard]] GameFilterConfigs getFilters() const;
};

#endif
