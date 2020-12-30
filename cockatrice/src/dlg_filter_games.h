#ifndef DLG_FILTER_GAMES_H
#define DLG_FILTER_GAMES_H

#include "gamesmodel.h"

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
    QCheckBox *showBuddiesOnlyGames;
    QCheckBox *showFullGames;
    QCheckBox *showGamesThatStarted;
    QCheckBox *showPasswordProtectedGames;
    QCheckBox *hideIgnoredUserGames;
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

    bool getShowFullGames() const;
    bool getShowGamesThatStarted() const;
    bool getShowPasswordProtectedGames() const;
    void setShowPasswordProtectedGames(bool _passwordProtectedGamesHidden);
    bool getShowBuddiesOnlyGames() const;
    void setShowBuddiesOnlyGames(bool _showBuddiesOnlyGames);
    bool getHideIgnoredUserGames() const;
    void setHideIgnoredUserGames(bool _hideIgnoredUserGames);
    QString getGameNameFilter() const;
    void setGameNameFilter(const QString &_gameNameFilter);
    QString getCreatorNameFilter() const;
    void setCreatorNameFilter(const QString &_creatorNameFilter);
    QSet<int> getGameTypeFilter() const;
    void setGameTypeFilter(const QSet<int> &_gameTypeFilter);
    int getMaxPlayersFilterMin() const;
    int getMaxPlayersFilterMax() const;
    void setMaxPlayersFilter(int _maxPlayersFilterMin, int _maxPlayersFilterMax);
    const QTime &getMaxGameAge() const;
    const QMap<QTime, QString> gameAgeMap;
    bool getShowOnlyIfSpectatorsCanWatch() const;
    bool getShowSpectatorPasswordProtected() const;
    bool getShowOnlyIfSpectatorsCanChat() const;
    bool getShowOnlyIfSpectatorsCanSeeHands() const;
};

#endif
