#include "userlist.h"
#include "tab_userlists.h"
#include "abstractclient.h"
#include "pixmapgenerator.h"
#include "userinfobox.h"
#include <QHeaderView>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QMenu>

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
	return data(2, Qt::UserRole).toString().toLower() < other.data(2, Qt::UserRole).toString().toLower();
}

UserList::UserList(TabUserLists *_tabUserLists, AbstractClient *_client, UserListType _type, QWidget *parent)
	: QGroupBox(parent), tabUserLists(_tabUserLists), client(_client), type(_type), onlineCount(0)
{
	itemDelegate = new UserListItemDelegate(this);
	
	userTree = new QTreeWidget;
	userTree->setColumnCount(3);
	userTree->header()->setResizeMode(QHeaderView::ResizeToContents);
	userTree->setHeaderHidden(true);
	userTree->setRootIsDecorated(false);
	userTree->setIconSize(QSize(20, 12));
	userTree->setItemDelegate(itemDelegate);
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
		retranslateUi();
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
			delete userTree->takeTopLevelItem(i);
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

void UserList::showContextMenu(const QPoint &pos, const QModelIndex &index)
{
	const QString &userName = index.sibling(index.row(), 2).data(Qt::UserRole).toString();
	
	QAction *aUserName = new QAction(userName, this);
	aUserName->setEnabled(false);
	QAction *aDetails = new QAction(tr("User &details"), this);
	QAction *aChat = new QAction(tr("Direct &chat"), this);
	QAction *aAddToBuddyList = new QAction(tr("Add to &buddy list"), this);
	QAction *aRemoveFromBuddyList = new QAction(tr("Remove from &buddy list"), this);
	QAction *aAddToIgnoreList = new QAction(tr("Remove from &ignore list"), this);
	QAction *aRemoveFromIgnoreList = new QAction(tr("Remove from &ignore list"), this);
	
	QMenu *menu = new QMenu(this);
	menu->addAction(aUserName);
	menu->addSeparator();
	menu->addAction(aDetails);
	menu->addAction(aChat);
	menu->addSeparator();
	if (tabUserLists->getBuddyList()->userInList(userName))
		menu->addAction(aRemoveFromBuddyList);
	else
		menu->addAction(aAddToBuddyList);
	if (tabUserLists->getIgnoreList()->userInList(userName))
		menu->addAction(aRemoveFromIgnoreList);
	else
		menu->addAction(aAddToIgnoreList);
	
	QAction *actionClicked = menu->exec(pos);
	if (actionClicked == aDetails) {
		UserInfoBox *infoWidget = new UserInfoBox(client, true, this, Qt::Dialog | Qt::WindowTitleHint | Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint);
		infoWidget->setAttribute(Qt::WA_DeleteOnClose);
		infoWidget->updateInfo(userName);
	} else if (actionClicked == aChat)
		emit openMessageDialog(userName, true);
	else if (actionClicked == aAddToBuddyList)
		emit addBuddy(userName);
	else if (actionClicked == aRemoveFromBuddyList)
		emit removeBuddy(userName);
	else if (actionClicked == aAddToIgnoreList)
		emit addIgnore(userName);
	else if (actionClicked == aRemoveFromIgnoreList)
		emit removeIgnore(userName);
	
	delete menu;
	delete aUserName;
	delete aDetails;
	delete aChat;
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
