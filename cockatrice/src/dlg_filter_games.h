#ifndef DLG_FILTER_GAMES_H
#define DLG_FILTER_GAMES_H

#include <QDialog>
#include <QSet>
#include <QMap>

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
public:
    DlgFilterGames(const QMap<int, QString> &allGameTypes, QWidget *parent = 0);
    
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
