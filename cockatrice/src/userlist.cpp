#include "userlist.h"
#include "tab_userlists.h"
#include "tab_supervisor.h"
#include "abstractclient.h"
#include "pixmapgenerator.h"
#include "userinfobox.h"
#include "protocol_items.h"
#include "gameselector.h"
#include <QHeaderView>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QMenu>
#include <QInputDialog>
#include <QLabel>
#include <QSpinBox>
#include <QRadioButton>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QMessageBox>

BanDialog::BanDialog(ServerInfo_User *info, QWidget *parent)
	: QDialog(parent)
{
	setAttribute(Qt::WA_DeleteOnClose);
	
	nameBanCheckBox = new QCheckBox(tr("ban &user name"));
	nameBanCheckBox->setChecked(true);
	nameBanEdit = new QLineEdit(info->getName());
	ipBanCheckBox = new QCheckBox(tr("ban &IP address"));
	ipBanCheckBox->setChecked(true);
	ipBanEdit = new QLineEdit(info->getAddress());
	QGridLayout *banTypeGrid = new QGridLayout;
	banTypeGrid->addWidget(nameBanCheckBox, 0, 0);
	banTypeGrid->addWidget(nameBanEdit, 0, 1);
	banTypeGrid->addWidget(ipBanCheckBox, 1, 0);
	banTypeGrid->addWidget(ipBanEdit, 1, 1);
	QGroupBox *banTypeGroupBox = new QGroupBox(tr("Ban type"));
	banTypeGroupBox->setLayout(banTypeGrid);
	
	permanentRadio = new QRadioButton(tr("&permanent ban"));
	temporaryRadio = new QRadioButton(tr("&temporary ban"));
	temporaryRadio->setChecked(true);
	connect(temporaryRadio, SIGNAL(toggled(bool)), this, SLOT(enableTemporaryEdits(bool)));
	daysLabel = new QLabel(tr("&Days:"));
	daysEdit = new QSpinBox;
	daysEdit->setMinimum(0);
	daysEdit->setValue(0);
	daysEdit->setMaximum(10000);
	daysLabel->setBuddy(daysEdit);
	hoursLabel = new QLabel(tr("&Hours:"));
	hoursEdit = new QSpinBox;
	hoursEdit->setMinimum(0);
	hoursEdit->setValue(0);
	hoursEdit->setMaximum(24);
	hoursLabel->setBuddy(hoursEdit);
	minutesLabel = new QLabel(tr("&Minutes:"));
	minutesEdit = new QSpinBox;
	minutesEdit->setMinimum(0);
	minutesEdit->setValue(5);
	minutesEdit->setMaximum(60);
	minutesLabel->setBuddy(minutesEdit);
	QGridLayout *durationLayout = new QGridLayout;
	durationLayout->addWidget(permanentRadio, 0, 0, 1, 6);
	durationLayout->addWidget(temporaryRadio, 1, 0, 1, 6);
	durationLayout->addWidget(daysLabel, 2, 0);
	durationLayout->addWidget(daysEdit, 2, 1);
	durationLayout->addWidget(hoursLabel, 2, 2);
	durationLayout->addWidget(hoursEdit, 2, 3);
	durationLayout->addWidget(minutesLabel, 2, 4);
	durationLayout->addWidget(minutesEdit, 2, 5);
	QGroupBox *durationGroupBox = new QGroupBox(tr("Duration of the ban"));
	durationGroupBox->setLayout(durationLayout);
	
	QLabel *reasonLabel = new QLabel(tr("Please enter the reason for the ban.\nThis is only saved for moderators and cannot be seen by the banned person."));
	reasonEdit = new QPlainTextEdit;
	
	QPushButton *okButton = new QPushButton(tr("&OK"));
	okButton->setAutoDefault(true);
	connect(okButton, SIGNAL(clicked()), this, SLOT(okClicked()));
	QPushButton *cancelButton = new QPushButton(tr("&Cancel"));
	connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
	
	QHBoxLayout *buttonLayout = new QHBoxLayout;
	buttonLayout->addStretch();
	buttonLayout->addWidget(okButton);
	buttonLayout->addWidget(cancelButton);
	
	QVBoxLayout *vbox = new QVBoxLayout;
	vbox->addWidget(banTypeGroupBox);
	vbox->addWidget(durationGroupBox);
	vbox->addWidget(reasonLabel);
	vbox->addWidget(reasonEdit);
	vbox->addLayout(buttonLayout);
	
	setLayout(vbox);
	setWindowTitle(tr("Ban user from server"));
}

void BanDialog::okClicked()
{
	if (!nameBanCheckBox->isChecked() && !ipBanCheckBox->isChecked()) {
		QMessageBox::critical(this, tr("Error"), tr("You have to select a name-based or IP-based ban, or both."));
		return;
	}
	accept();
}

void BanDialog::enableTemporaryEdits(bool enabled)
{
	daysLabel->setEnabled(enabled);
	daysEdit->setEnabled(enabled);
	hoursLabel->setEnabled(enabled);
	hoursEdit->setEnabled(enabled);
	minutesLabel->setEnabled(enabled);
	minutesEdit->setEnabled(enabled);
}

QString BanDialog::getBanName() const
{
	return nameBanCheckBox->isChecked() ? nameBanEdit->text() : QString();
}

QString BanDialog::getBanIP() const
{
	return ipBanCheckBox->isChecked() ? ipBanEdit->text() : QString();
}

int BanDialog::getMinutes() const
{
	return permanentRadio->isChecked() ? 0 : (daysEdit->value() * 24 * 60 + hoursEdit->value() * 60 + minutesEdit->value());
}

QString BanDialog::getReason() const
{
	return reasonEdit->toPlainText();
}

UserListItemDelegate::UserListItemDelegate(QObject *const parent)
	: QStyledItemDelegate(parent)
{
}

bool UserListItemDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
	if ((event->type() == QEvent::MouseButtonPress) && index.isValid()) {
		QMouseEvent *const mouseEvent = static_cast<QMouseEvent *>(event);
		if (mouseEvent->button() == Qt::RightButton) {
			static_cast<UserList *>(parent())->showContextMenu(mouseEvent->globalPos(), index);
			return true;
		}
	}
	return QStyledItemDelegate::editorEvent(event, model, option, index);
}

UserListTWI::UserListTWI()
	: QTreeWidgetItem(Type)
{
}

bool UserListTWI::operator<(const QTreeWidgetItem &other) const
{
	// Sort by online/offline
	if (data(0, Qt::UserRole + 1) != other.data(0, Qt::UserRole + 1))
		return data(0, Qt::UserRole + 1).toBool();
	
	// Sort by user level
	if (data(0, Qt::UserRole) != other.data(0, Qt::UserRole))
		return data(0, Qt::UserRole).toInt() > other.data(0, Qt::UserRole).toInt();
	
	// Sort by name
	return QString::localeAwareCompare(data(2, Qt::UserRole).toString(), other.data(2, Qt::UserRole).toString()) < 0;
}

UserList::UserList(TabSupervisor *_tabSupervisor, AbstractClient *_client, UserListType _type, QWidget *parent)
	: QGroupBox(parent), tabSupervisor(_tabSupervisor), client(_client), type(_type), onlineCount(0)
{
	itemDelegate = new UserListItemDelegate(this);
	
	userTree = new QTreeWidget;
	userTree->setColumnCount(3);
	userTree->header()->setResizeMode(QHeaderView::ResizeToContents);
	userTree->setHeaderHidden(true);
	userTree->setRootIsDecorated(false);
	userTree->setIconSize(QSize(20, 12));
	userTree->setItemDelegate(itemDelegate);
	userTree->setAlternatingRowColors(true);
	connect(userTree, SIGNAL(itemActivated(QTreeWidgetItem *, int)), this, SLOT(userClicked(QTreeWidgetItem *, int)));
	
	QVBoxLayout *vbox = new QVBoxLayout;
	vbox->addWidget(userTree);
	
	setLayout(vbox);
	
	retranslateUi();
}

void UserList::retranslateUi()
{
	switch (type) {
		case AllUsersList: titleStr = tr("Users online: %1"); break;
		case RoomList: titleStr = tr("Users in this room: %1"); break;
		case BuddyList: titleStr = tr("Buddies online: %1 / %2"); break;
		case IgnoreList: titleStr = tr("Ignored users online: %1 / %2"); break;
	}
	updateCount();
}

void UserList::processUserInfo(ServerInfo_User *user, bool online)
{
	QTreeWidgetItem *item = 0;
	for (int i = 0; i < userTree->topLevelItemCount(); ++i) {
		QTreeWidgetItem *temp = userTree->topLevelItem(i);
		if (temp->data(2, Qt::UserRole) == user->getName()) {
			item = temp;
			break;
		}
	}
	if (!item) {
		item = new UserListTWI;
		userTree->addTopLevelItem(item);
		if (online)
			++onlineCount;
		updateCount();
	}
	item->setData(0, Qt::UserRole, user->getUserLevel());
	item->setIcon(0, QIcon(UserLevelPixmapGenerator::generatePixmap(12, user->getUserLevel())));
	item->setIcon(1, QIcon(CountryPixmapGenerator::generatePixmap(12, user->getCountry())));
	item->setData(2, Qt::UserRole, user->getName());
	item->setData(2, Qt::DisplayRole, user->getName());
	
	item->setData(0, Qt::UserRole + 1, online);
	if (online)
		item->setData(2, Qt::ForegroundRole, QBrush());
	else
		item->setData(2, Qt::ForegroundRole, QBrush(Qt::gray));
}

bool UserList::deleteUser(const QString &userName)
{
	for (int i = 0; i < userTree->topLevelItemCount(); ++i)
		if (userTree->topLevelItem(i)->data(2, Qt::UserRole) == userName) {
			QTreeWidgetItem *item = userTree->takeTopLevelItem(i);
			if (item->data(0, Qt::UserRole + 1).toBool())
				--onlineCount;
			delete item;
			updateCount();
			return true;
		}
	
	return false;
}

void UserList::setUserOnline(QTreeWidgetItem *item, bool online)
{
	item->setData(0, Qt::UserRole + 1, online);
	
	if (online) {
		item->setData(2, Qt::ForegroundRole, QBrush());
		++onlineCount;
	} else {
		item->setData(2, Qt::ForegroundRole, QBrush(Qt::gray));
		--onlineCount;
	}
	updateCount();
}

void UserList::setUserOnline(const QString &userName, bool online)
{
	for (int i = 0; i < userTree->topLevelItemCount(); ++i) {
		QTreeWidgetItem *item = userTree->topLevelItem(i);
		if (item->data(2, Qt::UserRole) == userName) {
			setUserOnline(item, online);
			break;
		}
	}
}

void UserList::updateCount()
{
	QString str = titleStr;
	if ((type == BuddyList) || (type == IgnoreList))
		str = str.arg(onlineCount);
	setTitle(str.arg(userTree->topLevelItemCount()));
}

void UserList::userClicked(QTreeWidgetItem *item, int /*column*/)
{
	emit openMessageDialog(item->data(2, Qt::UserRole).toString(), true);
}

void UserList::gamesOfUserReceived(ProtocolResponse *resp)
{
	Command_GetGamesOfUser *command = static_cast<Command_GetGamesOfUser *>(sender());
	Response_GetGamesOfUser *response = qobject_cast<Response_GetGamesOfUser *>(resp);
	if (!response)
		return;
	
	QMap<int, GameTypeMap> gameTypeMap;
	QMap<int, QString> roomMap;
	const QList<ServerInfo_Room *> roomList = response->getRoomList();
	for (int i = 0; i < roomList.size(); ++i) {
		roomMap.insert(roomList[i]->getRoomId(), roomList[i]->getName());
		const QList<ServerInfo_GameType *> gameTypeList = roomList[i]->getGameTypeList();
		GameTypeMap tempMap;
		for (int j = 0; j < gameTypeList.size(); ++j)
			tempMap.insert(gameTypeList[j]->getGameTypeId(), gameTypeList[j]->getDescription());
		gameTypeMap.insert(roomList[i]->getRoomId(), tempMap);
	}
	
	GameSelector *selector = new GameSelector(client, tabSupervisor, 0, roomMap, gameTypeMap);
	const QList<ServerInfo_Game *> gameList = response->getGameList();
	for (int i = 0; i < gameList.size(); ++i)
		selector->processGameInfo(gameList[i]);
	
	selector->setWindowTitle(tr("%1's games").arg(command->getUserName()));
	selector->setAttribute(Qt::WA_DeleteOnClose);
	selector->show();
}

void UserList::banUser_processUserInfoResponse(ProtocolResponse *r)
{
	Response_GetUserInfo *response = qobject_cast<Response_GetUserInfo *>(r);
	if (!response)
		return;
	
	// The dialog needs to be non-modal in order to not block the event queue of the client.
	BanDialog *dlg = new BanDialog(response->getUserInfo(), this);
	connect(dlg, SIGNAL(accepted()), this, SLOT(banUser_dialogFinished()));
	dlg->show();
}

void UserList::banUser_dialogFinished()
{
	BanDialog *dlg = static_cast<BanDialog *>(sender());
	client->sendCommand(new Command_BanFromServer(dlg->getBanName(), dlg->getBanIP(), dlg->getMinutes(), dlg->getReason()));
}

void UserList::showContextMenu(const QPoint &pos, const QModelIndex &index)
{
	const QString &userName = index.sibling(index.row(), 2).data(Qt::UserRole).toString();
	ServerInfo_User::UserLevelFlags userLevel = static_cast<ServerInfo_User::UserLevelFlags>(index.sibling(index.row(), 0).data(Qt::UserRole).toInt());
	
	QAction *aUserName = new QAction(userName, this);
	aUserName->setEnabled(false);
	QAction *aDetails = new QAction(tr("User &details"), this);
	QAction *aChat = new QAction(tr("Direct &chat"), this);
	QAction *aShowGames = new QAction(tr("Show this user's &games"), this);
	QAction *aAddToBuddyList = new QAction(tr("Add to &buddy list"), this);
	QAction *aRemoveFromBuddyList = new QAction(tr("Remove from &buddy list"), this);
	QAction *aAddToIgnoreList = new QAction(tr("Add to &ignore list"), this);
	QAction *aRemoveFromIgnoreList = new QAction(tr("Remove from &ignore list"), this);
	QAction *aBan = new QAction(tr("Ban from &server"), this);
	
	QMenu *menu = new QMenu(this);
	menu->addAction(aUserName);
	menu->addSeparator();
	menu->addAction(aDetails);
	menu->addAction(aShowGames);
	menu->addAction(aChat);
	if ((userLevel & ServerInfo_User::IsRegistered) && (tabSupervisor->getUserLevel() & ServerInfo_User::IsRegistered)) {
		menu->addSeparator();
		if (tabSupervisor->getUserListsTab()->getBuddyList()->userInList(userName))
			menu->addAction(aRemoveFromBuddyList);
		else
			menu->addAction(aAddToBuddyList);
		if (tabSupervisor->getUserListsTab()->getIgnoreList()->userInList(userName))
			menu->addAction(aRemoveFromIgnoreList);
		else
			menu->addAction(aAddToIgnoreList);
	}
	if (!tabSupervisor->getAdminLocked()) {
		menu->addSeparator();
		menu->addAction(aBan);
	}
	if (userName == tabSupervisor->getUserInfo()->getName()) {
		aChat->setEnabled(false);
		aAddToBuddyList->setEnabled(false);
		aRemoveFromBuddyList->setEnabled(false);
		aAddToIgnoreList->setEnabled(false);
		aRemoveFromIgnoreList->setEnabled(false);
		aBan->setEnabled(false);
	}
	
	QAction *actionClicked = menu->exec(pos);
	if (actionClicked == aDetails) {
		UserInfoBox *infoWidget = new UserInfoBox(client, true, this, Qt::Dialog | Qt::WindowTitleHint | Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint);
		infoWidget->setAttribute(Qt::WA_DeleteOnClose);
		infoWidget->updateInfo(userName);
	} else if (actionClicked == aChat)
		emit openMessageDialog(userName, true);
	else if (actionClicked == aAddToBuddyList)
		client->sendCommand(new Command_AddToList("buddy", userName));
	else if (actionClicked == aRemoveFromBuddyList)
		client->sendCommand(new Command_RemoveFromList("buddy", userName));
	else if (actionClicked == aShowGames) {
		Command *cmd = new Command_GetGamesOfUser(userName);
		connect(cmd, SIGNAL(finished(ProtocolResponse *)), this, SLOT(gamesOfUserReceived(ProtocolResponse *)));
		client->sendCommand(cmd);
	} else if (actionClicked == aAddToIgnoreList)
		client->sendCommand(new Command_AddToList("ignore", userName));
	else if (actionClicked == aRemoveFromIgnoreList)
		client->sendCommand(new Command_RemoveFromList("ignore", userName));
	else if (actionClicked == aBan) {
		Command_GetUserInfo *command = new Command_GetUserInfo(userName);
		connect(command, SIGNAL(finished(ProtocolResponse *)), this, SLOT(banUser_processUserInfoResponse(ProtocolResponse *)));
		client->sendCommand(command);
	}
	
	delete menu;
	delete aUserName;
	delete aDetails;
	delete aChat;
	delete aAddToBuddyList;
	delete aRemoveFromBuddyList;
	delete aAddToIgnoreList;
	delete aRemoveFromIgnoreList;
	delete aBan;
}

bool UserList::userInList(const QString &userName) const
{
	for (int i = 0; i < userTree->topLevelItemCount(); ++i)
		if (userTree->topLevelItem(i)->data(2, Qt::UserRole) == userName)
			return true;
	return false;
}

void UserList::sortItems()
{
	userTree->sortItems(1, Qt::AscendingOrder);
}
