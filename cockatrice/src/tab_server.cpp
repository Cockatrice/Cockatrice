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
#include "pixmapgenerator.h"
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
			joinRoom(room->getRoomId());
	}
}

void RoomSelector::joinRoom(int id)
{
	Command_JoinRoom *command = new Command_JoinRoom(id);
	connect(command, SIGNAL(finished(ProtocolResponse *)), this, SLOT(joinFinished(ProtocolResponse *)));
	client->sendCommand(command);
}

void RoomSelector::joinClicked()
{
	QTreeWidgetItem *twi = roomList->currentItem();
	if (!twi)
		return;
	
	joinRoom(twi->data(0, Qt::UserRole).toInt());
}

void RoomSelector::joinFinished(ProtocolResponse *r)
{
	if (r->getResponseCode() != RespOk)
		return;
	Response_JoinRoom *resp = qobject_cast<Response_JoinRoom *>(r);
	if (!resp)
		return;
	
	emit roomJoined(resp->getRoomInfo());
}

UserInfoBox::UserInfoBox(ServerInfo_User *userInfo, QWidget *parent)
	: QWidget(parent)
{
	avatarLabel = new QLabel;
	nameLabel = new QLabel;
	QFont nameFont = nameLabel->font();
	nameFont.setBold(true);
	nameFont.setPointSize(nameFont.pointSize() * 1.5);
	nameLabel->setFont(nameFont);
	countryLabel1 = new QLabel;
	countryLabel2 = new QLabel;
	userLevelLabel1 = new QLabel;
	userLevelLabel2 = new QLabel;
	userLevelLabel3 = new QLabel;
	
	QGridLayout *mainLayout = new QGridLayout;
	mainLayout->addWidget(avatarLabel, 0, 0, 3, 1, Qt::AlignCenter);
	mainLayout->addWidget(nameLabel, 0, 1, 1, 3);
	mainLayout->addWidget(countryLabel1, 1, 1, 1, 1);
	mainLayout->addWidget(countryLabel2, 1, 2, 1, 2);
	mainLayout->addWidget(userLevelLabel1, 2, 1, 1, 1);
	mainLayout->addWidget(userLevelLabel2, 2, 2, 1, 1);
	mainLayout->addWidget(userLevelLabel3, 2, 3, 1, 1);
	mainLayout->setColumnMinimumWidth(0, 80);
	mainLayout->setColumnStretch(3, 10);
	
	setLayout(mainLayout);
	
	updateInfo(userInfo);
}

void UserInfoBox::retranslateUi()
{
	countryLabel1->setText(tr("Location:"));
	userLevelLabel1->setText(tr("User level:"));
}

void UserInfoBox::updateInfo(ServerInfo_User *user)
{
	int userLevel = user->getUserLevel();
	
	QPixmap avatarPixmap;
	if (!avatarPixmap.loadFromData(user->getAvatarBmp()))
		avatarPixmap = UserLevelPixmapGenerator::generatePixmap(64, userLevel);
	avatarLabel->setPixmap(avatarPixmap);
	
	nameLabel->setText(user->getName());
	countryLabel2->setPixmap(CountryPixmapGenerator::generatePixmap(15, user->getCountry()));
	userLevelLabel2->setPixmap(UserLevelPixmapGenerator::generatePixmap(15, userLevel));
	QString userLevelText;
	if (userLevel & ServerInfo_User::IsAdmin)
		userLevelText = tr("Administrator");
	else if (userLevel & ServerInfo_User::IsJudge)
		userLevelText = tr("Judge");
	else if (userLevel & ServerInfo_User::IsRegistered)
		userLevelText = tr("Registered user");
	else
		userLevelText = tr("Unregistered user");
	userLevelLabel3->setText(userLevelText);
}

TabServer::TabServer(AbstractClient *_client, ServerInfo_User *userInfo, QWidget *parent)
	: Tab(parent), client(_client)
{
	roomSelector = new RoomSelector(client);
	serverInfoBox = new QTextBrowser;
	userInfoBox = new UserInfoBox(userInfo);
	userList = new UserList(true);
	
	connect(roomSelector, SIGNAL(roomJoined(ServerInfo_Room *)), this, SIGNAL(roomJoined(ServerInfo_Room *)));
	connect(userList, SIGNAL(openMessageDialog(const QString &, bool)), this, SIGNAL(openMessageDialog(const QString &, bool)));
	
	connect(client, SIGNAL(userJoinedEventReceived(Event_UserJoined *)), this, SLOT(processUserJoinedEvent(Event_UserJoined *)));
	connect(client, SIGNAL(userLeftEventReceived(Event_UserLeft *)), this, SLOT(processUserLeftEvent(Event_UserLeft *)));
	connect(client, SIGNAL(serverMessageEventReceived(Event_ServerMessage *)), this, SLOT(processServerMessageEvent(Event_ServerMessage *)));
	
	Command_ListUsers *cmd = new Command_ListUsers;
	connect(cmd, SIGNAL(finished(ProtocolResponse *)), this, SLOT(processListUsersResponse(ProtocolResponse *)));
	client->sendCommand(cmd);
	
	QVBoxLayout *vbox = new QVBoxLayout;
	vbox->addWidget(roomSelector);
	vbox->addWidget(serverInfoBox);
	
	QVBoxLayout *vbox2 = new QVBoxLayout;
	vbox2->addWidget(userInfoBox);
	vbox2->addWidget(userList);
	
	QHBoxLayout *mainLayout = new QHBoxLayout;
	mainLayout->addLayout(vbox, 3);
	mainLayout->addLayout(vbox2, 1);
	
	setLayout(mainLayout);
}

void TabServer::retranslateUi()
{
	roomSelector->retranslateUi();
	userInfoBox->retranslateUi();
	userList->retranslateUi();
}

void TabServer::processServerMessageEvent(Event_ServerMessage *event)
{
	serverInfoBox->setHtml(event->getMessage());
}

void TabServer::processListUsersResponse(ProtocolResponse *response)
{
	Response_ListUsers *resp = qobject_cast<Response_ListUsers *>(response);
	if (!resp)
		return;
	
	const QList<ServerInfo_User *> &respList = resp->getUserList();
	for (int i = 0; i < respList.size(); ++i)
		userList->processUserInfo(respList[i]);
	
	userList->sortItems();
}

void TabServer::processUserJoinedEvent(Event_UserJoined *event)
{
	userList->processUserInfo(event->getUserInfo());
	userList->sortItems();
}

void TabServer::processUserLeftEvent(Event_UserLeft *event)
{
	if (userList->deleteUser(event->getUserName()))
		emit userLeft(event->getUserName());
}