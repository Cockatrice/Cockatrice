#ifndef DLG_FILTER_GAMES_H
#define DLG_FILTER_GAMES_H

#include <QDialog>
#include <QSet>
#include <QMap>
#include "gamesmodel.h"

class QCheckBox;
class QLineEdit;
class QSpinBox;

class DlgFilterGames : public QDialog {
    Q_OBJECT
private:
    QCheckBox *unavailableGamesVisibleCheckBox;
    QCheckBox *passwordProtectedGamesVisibleCheckBox;
    QLineEdit *gameNameFilterEdit;
    QLineEdit *creatorNameFilterEdit;
    QMap<int, QCheckBox *> gameTypeFilterCheckBoxes;
    QSpinBox *maxPlayersFilterMinSpinBox;
    QSpinBox *maxPlayersFilterMaxSpinBox;

    const QMap<int, QString> &allGameTypes;
    // This needs a const someplace
    GamesProxyModel *gamesProxyModel;

    /*
     * The game type might contain special characters, so to use it in
     * QSettings we just hash it.
     */
    QString hashGameType(const QString &gameType) const;
private slots:
    void actOk();
public:
    DlgFilterGames(const QMap<int, QString> &_allGameTypes, GamesProxyModel *_gamesProxyModel, QWidget *parent = 0);

    bool getUnavailableGamesVisible() const;
    void setUnavailableGamesVisible(bool _unavailableGamesVisible);
    bool getPasswordProtectedGamesVisible() const;
    void setPasswordProtectedGamesVisible(bool _passwordProtectedGamesVisible);
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
