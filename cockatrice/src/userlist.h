#ifndef USERLIST_H
#define USERLIST_H

#include <QGroupBox>
#include <QTreeWidgetItem>
#include <QItemDelegate>

class QTreeWidget;
class ServerInfo_User;
class AbstractClient;

class UserListItemDelegate : public QItemDelegate {
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
private:
	AbstractClient *client;
	QTreeWidget *userTree;
	UserListItemDelegate *itemDelegate;
	bool global;
	QString titleStr;
	void updateCount();
private slots:
	void userClicked(QTreeWidgetItem *item, int column);
signals:
	void openMessageDialog(const QString &userName, bool focus);
public:
	UserList(AbstractClient *_client, bool _global, QWidget *parent = 0);
	void retranslateUi();
	void processUserInfo(ServerInfo_User *user);
	bool deleteUser(const QString &userName);
	void showContextMenu(const QPoint &pos, const QModelIndex &index);
	void sortItems();
};

#endif