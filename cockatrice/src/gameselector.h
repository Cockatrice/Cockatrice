#ifndef GAMESELECTOR_H
#define GAMESELECTOR_H

#include <QGroupBox>
#include "gametypemap.h"

class QTreeView;
class GamesModel;
class GamesProxyModel;
class QPushButton;
class QCheckBox;
class AbstractClient;
class TabSupervisor;
class TabRoom;
class ServerInfo_Game;
class Response;

class GameSelector : public QGroupBox {
    Q_OBJECT
private slots:
    void actSetFilter();
    void actClearFilter();
    void actCreate();
    void actJoin();
    void actSelectedGameChanged(const QModelIndex &current, const QModelIndex &previous);
    void checkResponse(const Response &response);
signals:
    void gameJoined(int gameId);
private:
    AbstractClient *client;
    const TabSupervisor *tabSupervisor;
    TabRoom *room;

    QTreeView *gameListView;
    GamesModel *gameListModel;
    GamesProxyModel *gameListProxyModel;
    QPushButton *filterButton, *clearFilterButton, *createButton, *joinButton, *spectateButton;
    GameTypeMap gameTypeMap;
public:
    GameSelector(AbstractClient *_client, const TabSupervisor *_tabSupervisor, TabRoom *_room, const QMap<int, QString> &_rooms, const QMap<int, GameTypeMap> &_gameTypes, const bool restoresettings, const bool showfilters, QWidget *parent = 0);
    void retranslateUi();
    void processGameInfo(const ServerInfo_Game &info);
};

#endif
