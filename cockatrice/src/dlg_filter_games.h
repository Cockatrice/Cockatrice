#ifndef DLG_FILTER_GAMES_H
#define DLG_FILTER_GAMES_H

#include <QDialog>
#include <QSet>
#include <QMap>
#include <QCheckBox>
#include "gamesmodel.h"

class QCheckBox;
class QLineEdit;
class QSpinBox;

class DlgFilterGames : public QDialog {
    Q_OBJECT
private:
    QCheckBox *showBuddiesOnlyGames;
    QCheckBox *unavailableGamesVisibleCheckBox;
    QCheckBox *showPasswordProtectedGames;
    QLineEdit *gameNameFilterEdit;
    QLineEdit *creatorNameFilterEdit;
    QMap<int, QCheckBox *> gameTypeFilterCheckBoxes;
    QSpinBox *maxPlayersFilterMinSpinBox;
    QSpinBox *maxPlayersFilterMaxSpinBox;

    const QMap<int, QString> &allGameTypes;
    const GamesProxyModel *gamesProxyModel;

private slots:
    void actOk();
public:
    DlgFilterGames(const QMap<int, QString> &_allGameTypes, const GamesProxyModel *_gamesProxyModel, QWidget *parent = 0);

    bool getUnavailableGamesVisible() const;
    void setUnavailableGamesVisible(bool _unavailableGamesVisible);
    bool getShowPasswordProtectedGames() const;
    void setShowPasswordProtectedGames(bool _passwordProtectedGamesHidden);
    bool getShowBuddiesOnlyGames() const;
    void setShowBuddiesOnlyGames(bool _showBuddiesOnlyGames);
    QString getGameNameFilter() const;
    void setGameNameFilter(const QString &_gameNameFilter);
    QString getCreatorNameFilter() const;
    void setCreatorNameFilter(const QString &_creatorNameFilter);
    QSet<int> getGameTypeFilter() const;
    void setGameTypeFilter(const QSet<int> &_gameTypeFilter);
    int getMaxPlayersFilterMin() const;
    int getMaxPlayersFilterMax() const;
    void setMaxPlayersFilter(int _maxPlayersFilterMin, int _maxPlayersFilterMax);
};

#endif
