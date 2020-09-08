#ifndef GAMESELECTOR_H
#define GAMESELECTOR_H

#include "gametypemap.h"

#include <QGroupBox>
#include <common/pb/event_add_to_list.pb.h>
#include <common/pb/event_remove_from_list.pb.h>

class QTreeView;
class GamesModel;
class GamesProxyModel;
class QPushButton;
class QCheckBox;
class QLabel;
class AbstractClient;
class TabSupervisor;
class TabRoom;
class ServerInfo_Game;
class Response;

class GameSelector : public QGroupBox
{
    Q_OBJECT
private slots:
    void actSetFilter();
    void actClearFilter();
    void actCreate();
    void actJoin();
    void actSelectedGameChanged(const QModelIndex &current, const QModelIndex &previous);
    void checkResponse(const Response &response);

    void ignoreListReceived(const QList<ServerInfo_User> &_ignoreList);
    void processAddToListEvent(const Event_AddToList &event);
    void processRemoveFromListEvent(const Event_RemoveFromList &event);
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
    const bool showFilters;
    GameTypeMap gameTypeMap;

    void updateTitle();

public:
    GameSelector(AbstractClient *_client,
                 const TabSupervisor *_tabSupervisor,
                 TabRoom *_room,
                 const QMap<int, QString> &_rooms,
                 const QMap<int, GameTypeMap> &_gameTypes,
                 const bool restoresettings,
                 const bool _showfilters,
                 QWidget *parent = nullptr);
    void retranslateUi();
    void processGameInfo(const ServerInfo_Game &info);
};

#endif
