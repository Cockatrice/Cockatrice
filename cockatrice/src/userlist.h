#ifndef USERLIST_H
#define USERLIST_H

#include <QDialog>
#include <QGroupBox>
#include <QComboBox>
#include <QTreeWidgetItem>
#include <QStyledItemDelegate>
#include "user_level.h"
#include "pb/moderator_commands.pb.h"

class QTreeWidget;
class ServerInfo_User;
class AbstractClient;
class TabSupervisor;
class QLabel;
class QCheckBox;
class QSpinBox;
class QRadioButton;
class QPlainTextEdit;
class Response;
class CommandContainer;
class UserContextMenu;

class BanDialog : public QDialog {
    Q_OBJECT
private:
    QLabel *daysLabel, *hoursLabel, *minutesLabel;
    QCheckBox *nameBanCheckBox, *ipBanCheckBox, *idBanCheckBox;
    QLineEdit *nameBanEdit, *ipBanEdit, *idBanEdit;
    QSpinBox *daysEdit, *hoursEdit, *minutesEdit;
    QRadioButton *permanentRadio, *temporaryRadio;
    QPlainTextEdit *reasonEdit, *visibleReasonEdit;
private slots:
    void okClicked();
    void enableTemporaryEdits(bool enabled);
public:
    BanDialog(const ServerInfo_User &info, QWidget *parent = 0);
    QString getBanName() const;
    QString getBanIP() const;
    QString getBanId() const;
    int getMinutes() const;
    QString getReason() const;
    QString getVisibleReason() const;
};

class WarningDialog : public QDialog {
    Q_OBJECT
private:
    QLabel *descriptionLabel;
    QLineEdit *nameWarning;
    QComboBox *warningOption;
    QLineEdit * warnClientID;
private slots:
    void okClicked();
public:
    WarningDialog(const QString userName, const QString clientID, QWidget *parent = 0);
    QString getName() const;
    QString getWarnID() const;
    QString getReason() const;
    void addWarningOption(const QString warning);
};

class UserListItemDelegate : public QStyledItemDelegate {
public:
    UserListItemDelegate(QObject *const parent);
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index);
};

class UserListTWI : public QTreeWidgetItem {
private:
    ServerInfo_User userInfo;
public:
    UserListTWI(const ServerInfo_User &_userInfo);
    const ServerInfo_User &getUserInfo() const { return userInfo; }
    void setUserInfo(const ServerInfo_User &_userInfo);
    void setOnline(bool online);
    bool operator<(const QTreeWidgetItem &other) const;
};

class UserList : public QGroupBox {
    Q_OBJECT
public:
    enum UserListType { AllUsersList, RoomList, BuddyList, IgnoreList };
private:
    QMap<QString, UserListTWI *> users;
    TabSupervisor *tabSupervisor;
    AbstractClient *client;
    UserListType type;
    QTreeWidget *userTree;
    UserListItemDelegate *itemDelegate;
    UserContextMenu *userContextMenu;
    int onlineCount;
    QString titleStr;
    void updateCount();
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
    void processUserInfo(const ServerInfo_User &user, bool online);
    bool deleteUser(const QString &userName);
    void setUserOnline(const QString &userName, bool online);
    const QMap<QString, UserListTWI *> &getUsers() const { return users; }
    void showContextMenu(const QPoint &pos, const QModelIndex &index);
    void sortItems();
};

#endif
