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
#include "protocol.h"
#include "protocol_items.h"
#include "userlist.h"
#include "userinfobox.h"
#include <QDebug>

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
	
	connect(client, SIGNAL(listRoomsEventReceived(Event_ListRooms *)), this, SLOT(processListRoomsEvent(Event_ListRooms *)));
	client->sendCommand(new Command_ListRooms);
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

void RoomSelector::processListRoomsEvent(Event_ListRooms *event)
{
	const QList<ServerInfo_Room *> &roomsToUpdate = event->getRoomList();
	for (int i = 0; i < roomsToUpdate.size(); ++i) {
		ServerInfo_Room *room = roomsToUpdate[i];
		
		for (int j = 0; j < roomList->topLevelItemCount(); ++j) {
		  	QTreeWidgetItem *twi = roomList->topLevelItem(j);
			if (twi->data(0, Qt::UserRole).toInt() == room->getRoomId()) {
				twi->setData(0, Qt::DisplayRole, room->getName());
				twi->setData(1, Qt::DisplayRole, room->getDescription());
				twi->setData(2, Qt::DisplayRole, room->getPlayerCount());
				twi->setData(3, Qt::DisplayRole, room->getGameCount());
				return;
			}
		}
		QTreeWidgetItem *twi = new QTreeWidgetItem;
		twi->setData(0, Qt::UserRole, room->getRoomId());
		twi->setData(0, Qt::DisplayRole, room->getName());
		twi->setData(1, Qt::DisplayRole, room->getDescription());
		twi->setData(2, Qt::DisplayRole, room->getPlayerCount());
		twi->setData(3, Qt::DisplayRole, room->getGameCount());
		twi->setTextAlignment(2, Qt::AlignRight);
		twi->setTextAlignment(3, Qt::AlignRight);
		roomList->addTopLevelItem(twi);
		if (room->getAutoJoin())
			joinRoom(room->getRoomId(), false);
	}
}

void RoomSelector::joinRoom(int id, bool setCurrent)
{
	Command_JoinRoom *command = new Command_JoinRoom(id);
	command->setExtraData(setCurrent);
	connect(command, SIGNAL(finished(ProtocolResponse *)), this, SLOT(joinFinished(ProtocolResponse *)));
	client->sendCommand(command);
}

void RoomSelector::joinClicked()
{
	QTreeWidgetItem *twi = roomList->currentItem();
	if (!twi)
		return;
	
	joinRoom(twi->data(0, Qt::UserRole).toInt(), true);
}

void RoomSelector::joinFinished(ProtocolResponse *r)
{
	if (r->getResponseCode() != RespOk)
		return;
	Response_JoinRoom *resp = qobject_cast<Response_JoinRoom *>(r);
	if (!resp)
		return;
	
	emit roomJoined(resp->getRoomInfo(), static_cast<Command *>(sender())->getExtraData().toBool());
}

TabServer::TabServer(TabSupervisor *_tabSupervisor, AbstractClient *_client, QWidget *parent)
	: Tab(_tabSupervisor, parent), client(_client)
{
	roomSelector = new RoomSelector(client);
	serverInfoBox = new QTextBrowser;
	serverInfoBox->setOpenExternalLinks(true);
	
	connect(roomSelector, SIGNAL(roomJoined(ServerInfo_Room *, bool)), this, SIGNAL(roomJoined(ServerInfo_Room *, bool)));
	
	connect(client, SIGNAL(serverMessageEventReceived(Event_ServerMessage *)), this, SLOT(processServerMessageEvent(Event_ServerMessage *)));
	
	QVBoxLayout *vbox = new QVBoxLayout;
	vbox->addWidget(roomSelector);
	vbox->addWidget(serverInfoBox);
	
	setLayout(vbox);
}

void TabServer::retranslateUi()
{
	roomSelector->retranslateUi();
}

void TabServer::processServerMessageEvent(Event_ServerMessage *event)
{
	serverInfoBox->setHtml(event->getMessage());
	emit userEvent();
}
