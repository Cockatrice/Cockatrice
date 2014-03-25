#include <QLabel>
#include <QTreeView>
#include <QCheckBox>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QMessageBox>
#include <QLineEdit>
#include <QHeaderView>
#include <QInputDialog>
#include "tab_server.h"
#include "abstractclient.h"
#include "userlist.h"
#include "userinfobox.h"
#include <QDebug>

#include "pending_command.h"
#include "pb/session_commands.pb.h"
#include "pb/event_list_rooms.pb.h"
#include "pb/event_server_message.pb.h"
#include "pb/response_join_room.pb.h"

RoomSelector::RoomSelector(AbstractClient *_client, QWidget *parent)
    : QGroupBox(parent), client(_client)
{
    roomList = new QTreeWidget;
    roomList->setRootIsDecorated(false);
    roomList->setColumnCount(4);
    roomList->header()->setStretchLastSection(false);
    roomList->header()->setResizeMode(0, QHeaderView::ResizeToContents);
    roomList->header()->setResizeMode(1, QHeaderView::Stretch);
    roomList->header()->setResizeMode(2, QHeaderView::ResizeToContents);
    roomList->header()->setResizeMode(3, QHeaderView::ResizeToContents);
    
    joinButton = new QPushButton;
    connect(joinButton, SIGNAL(clicked()), this, SLOT(joinClicked()));
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch();
    buttonLayout->addWidget(joinButton);
    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->addWidget(roomList);
    vbox->addLayout(buttonLayout);
    
    retranslateUi();
    setLayout(vbox);
    
    connect(client, SIGNAL(listRoomsEventReceived(const Event_ListRooms &)), this, SLOT(processListRoomsEvent(const Event_ListRooms &)));
    client->sendCommand(client->prepareSessionCommand(Command_ListRooms()));
}

void RoomSelector::retranslateUi()
{
    setTitle(tr("Rooms"));
    joinButton->setText(tr("Joi&n"));

    QTreeWidgetItem *header = roomList->headerItem();
    header->setText(0, tr("Room"));
    header->setText(1, tr("Description"));
    header->setText(2, tr("Players"));
    header->setText(3, tr("Games"));
    header->setTextAlignment(2, Qt::AlignRight);
    header->setTextAlignment(3, Qt::AlignRight);
}

void RoomSelector::processListRoomsEvent(const Event_ListRooms &event)
{
    const int roomListSize = event.room_list_size();
    for (int i = 0; i < roomListSize; ++i) {
        const ServerInfo_Room &room = event.room_list(i);
        
        for (int j = 0; j < roomList->topLevelItemCount(); ++j) {
              QTreeWidgetItem *twi = roomList->topLevelItem(j);
            if (twi->data(0, Qt::UserRole).toInt() == room.room_id()) {
                if (room.has_name())
                    twi->setData(0, Qt::DisplayRole, QString::fromStdString(room.name()));
                if (room.has_description())
                    twi->setData(1, Qt::DisplayRole, QString::fromStdString(room.description()));
                if (room.has_player_count())
                    twi->setData(2, Qt::DisplayRole, room.player_count());
                if (room.has_game_count())
                    twi->setData(3, Qt::DisplayRole, room.game_count());
                return;
            }
        }
        QTreeWidgetItem *twi = new QTreeWidgetItem;
        twi->setData(0, Qt::UserRole, room.room_id());
        if (room.has_name())
            twi->setData(0, Qt::DisplayRole, QString::fromStdString(room.name()));
        if (room.has_description())
            twi->setData(1, Qt::DisplayRole, QString::fromStdString(room.description()));
        twi->setData(2, Qt::DisplayRole, room.player_count());
        twi->setData(3, Qt::DisplayRole, room.game_count());
        twi->setTextAlignment(2, Qt::AlignRight);
        twi->setTextAlignment(3, Qt::AlignRight);
        
        roomList->addTopLevelItem(twi);
        if (room.has_auto_join())
            if (room.auto_join())
                joinRoom(room.room_id(), false);
    }
}

void RoomSelector::joinRoom(int id, bool setCurrent)
{
    Command_JoinRoom cmd;
    cmd.set_room_id(id);
    
    PendingCommand *pend = client->prepareSessionCommand(cmd);
    pend->setExtraData(setCurrent);
    connect(pend, SIGNAL(finished(Response, CommandContainer, QVariant)), this, SLOT(joinFinished(Response, CommandContainer, QVariant)));
    
    client->sendCommand(pend);
}

void RoomSelector::joinClicked()
{
    QTreeWidgetItem *twi = roomList->currentItem();
    if (!twi)
        return;
    
    joinRoom(twi->data(0, Qt::UserRole).toInt(), true);
}

void RoomSelector::joinFinished(const Response &r, const CommandContainer & /*commandContainer*/, const QVariant &extraData)
{
    if (r.response_code() != Response::RespOk)
        return;
    const Response_JoinRoom &resp = r.GetExtension(Response_JoinRoom::ext);
    
    emit roomJoined(resp.room_info(), extraData.toBool());
}

TabServer::TabServer(TabSupervisor *_tabSupervisor, AbstractClient *_client, QWidget *parent)
    : Tab(_tabSupervisor, parent), client(_client)
{
    roomSelector = new RoomSelector(client);
    serverInfoBox = new QTextBrowser;
    serverInfoBox->setOpenExternalLinks(true);
    
    connect(roomSelector, SIGNAL(roomJoined(const ServerInfo_Room &, bool)), this, SIGNAL(roomJoined(const ServerInfo_Room &, bool)));
    
    connect(client, SIGNAL(serverMessageEventReceived(const Event_ServerMessage &)), this, SLOT(processServerMessageEvent(const Event_ServerMessage &)));
    
    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->addWidget(roomSelector);
    vbox->addWidget(serverInfoBox);
    
    setLayout(vbox);
}

void TabServer::retranslateUi()
{
    roomSelector->retranslateUi();
}

void TabServer::processServerMessageEvent(const Event_ServerMessage &event)
{
    serverInfoBox->setHtml(QString::fromStdString(event.message()));
    emit userEvent();
}
