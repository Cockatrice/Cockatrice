#ifndef USERLIST_H
#define USERLIST_H

#include <QGroupBox>
#include <QTreeWidgetItem>
#include <QStyledItemDelegate>

class QTreeWidget;
class ServerInfo_User;
class AbstractClient;
class TabSupervisor;

class UserListItemDelegate : public QStyledItemDelegate {
public:
	UserListItemDelegate(QObject *const parent);
	bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index);
};

class UserListTWI : public QTreeWidgetItem {
public:
	UserListTWI();
	bool operator<(const QTreeWidgetItem &other) const;
};

class UserList : public QGroupBox {
	Q_OBJECT
public:
	enum UserListType { AllUsersList, RoomList, BuddyList, IgnoreList };
private:
	TabSupervisor *tabSupervisor;
	AbstractClient *client;
	UserListType type;
	QTreeWidget *userTree;
	UserListItemDelegate *itemDelegate;
	int onlineCount;
	QString titleStr;
	void updateCount();
	void setUserOnline(QTreeWidgetItem *user, bool online);
private slots:
	void userClicked(QTreeWidgetItem *item, int column);
signals:
	void openMessageDialog(const QString &userName, bool focus);
	void addBuddy(const QString &userName);
	void removeBuddy(const QString &userName);
	void addIgnore(const QString &userName);
	void removeIgnore(const QString &userName);
public:
	UserList(TabSupervisor *_tabSupervisor, AbstractClient *_client, UserListType _type, QWidget *parent = 0);
	void retranslateUi();
	void processUserInfo(ServerInfo_User *user, bool online);
	bool deleteUser(const QString &userName);
	void setUserOnline(const QString &userName, bool online);
	bool userInList(const QString &userName) const;
	void showContextMenu(const QPoint &pos, const QModelIndex &index);
	void sortItems();
};

#endif