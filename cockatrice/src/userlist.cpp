#include "userlist.h"
#include "abstractclient.h"
#include "pixmapgenerator.h"
#include <QHeaderView>
#include <QVBoxLayout>

UserListTWI::UserListTWI()
	: QTreeWidgetItem(Type)
{
}

bool UserListTWI::operator<(const QTreeWidgetItem &other) const
{
	// Equal user level => sort by name
	if (data(0, Qt::UserRole) == other.data(0, Qt::UserRole))
		return data(2, Qt::UserRole).toString().toLower() < other.data(2, Qt::UserRole).toString().toLower();
	// Else sort by user level
	return data(0, Qt::UserRole).toInt() > other.data(0, Qt::UserRole).toInt();
}

UserList::UserList(bool _global, QWidget *parent)
	: QGroupBox(parent), global(_global)
{
	userTree = new QTreeWidget;
	userTree->setColumnCount(3);
	userTree->header()->setResizeMode(QHeaderView::ResizeToContents);
	userTree->setHeaderHidden(true);
	userTree->setRootIsDecorated(false);
	userTree->setIconSize(QSize(20, 12));
	connect(userTree, SIGNAL(itemActivated(QTreeWidgetItem *, int)), this, SLOT(userClicked(QTreeWidgetItem *, int)));
	
	QVBoxLayout *vbox = new QVBoxLayout;
	vbox->addWidget(userTree);
	
	setLayout(vbox);
	
	retranslateUi();
}

void UserList::retranslateUi()
{
	titleStr = global ? tr("Users online: %1") : tr("Users in this room: %1");
	updateCount();
}

void UserList::processUserInfo(ServerInfo_User *user)
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

void UserList::updateCount()
{
	setTitle(titleStr.arg(userTree->topLevelItemCount()));
}

void UserList::userClicked(QTreeWidgetItem *item, int /*column*/)
{
	emit openMessageDialog(item->data(2, Qt::UserRole).toString(), true);
}

void UserList::sortItems()
{
	userTree->sortItems(1, Qt::AscendingOrder);
}
