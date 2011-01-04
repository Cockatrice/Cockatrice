#ifndef USERLIST_H
#define USERLIST_H

#include <QGroupBox>
#include <QTreeWidgetItem>

class QTreeWidget;
class ServerInfo_User;

class UserListTWI : public QTreeWidgetItem {
public:
	UserListTWI();
	bool operator<(const QTreeWidgetItem &other) const;
};

class UserList : public QGroupBox {
	Q_OBJECT
private:
	QTreeWidget *userTree;
	bool global;
	QString titleStr;
	void updateCount();
private slots:
	void userClicked(QTreeWidgetItem *item, int column);
signals:
	void openMessageDialog(const QString &userName, bool focus);
public:
	UserList(bool _global, QWidget *parent = 0);
	void retranslateUi();
	void processUserInfo(ServerInfo_User *user);
	bool deleteUser(const QString &userName);
	void sortItems();
};

#endif