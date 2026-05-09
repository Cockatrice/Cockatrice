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
    const QMap<QTime, QString> gameAgeMap;

    [[nodiscard]] QStringList getCreatorNameFilters() const;
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
