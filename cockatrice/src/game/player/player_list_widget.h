/**
 * @file player_list_widget.h
 * @ingroup GameWidgets
 * @brief TODO: Document this.
 */

#ifndef PLAYERLISTWIDGET_H
#define PLAYERLISTWIDGET_H

#include "player.h"

#include <QIcon>
#include <QMap>
#include <QStyledItemDelegate>
#include <QTreeWidget>

class ServerInfo_PlayerProperties;
class TabSupervisor;
class AbstractClient;
class AbstractGame;
class UserContextMenu;

class PlayerListItemDelegate : public QStyledItemDelegate
{
public:
    explicit PlayerListItemDelegate(QObject *parent);
    bool editorEvent(QEvent *event,
                     QAbstractItemModel *model,
                     const QStyleOptionViewItem &option,
                     const QModelIndex &index) override;
};

class PlayerListTWI : public QTreeWidgetItem
{
public:
    PlayerListTWI();
    bool operator<(const QTreeWidgetItem &other) const override;
};

class PlayerListWidget : public QTreeWidget
{
    Q_OBJECT
private:
    PlayerListItemDelegate *itemDelegate;
    QMap<int, QTreeWidgetItem *> players;
    TabSupervisor *tabSupervisor;
    AbstractClient *client;
    AbstractGame *game;
    UserContextMenu *userContextMenu;
    QIcon readyIcon, notReadyIcon, concededIcon, playerIcon, judgeIcon, spectatorIcon, lockIcon;
    bool gameStarted;
signals:
    void openMessageDialog(const QString &userName, bool focus);

public:
    PlayerListWidget(TabSupervisor *_tabSupervisor,
                     AbstractClient *_client,
                     AbstractGame *_game,
                     QWidget *parent = nullptr);
    void retranslateUi();
    void setActivePlayer(int playerId);
    void setGameStarted(bool _gameStarted, bool resuming);
    void showContextMenu(const QPoint &pos, const QModelIndex &index);

public slots:
    void addPlayer(const ServerInfo_PlayerProperties &player);
    void removePlayer(int playerId);
    void updatePlayerProperties(const ServerInfo_PlayerProperties &prop, int playerId = -1);
};

#endif
