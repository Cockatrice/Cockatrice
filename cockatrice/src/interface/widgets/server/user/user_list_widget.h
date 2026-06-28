/**
 * @file user_list_widget.h
 * @ingroup Lobby
 */
//! \todo Document this file.

#ifndef USERLIST_H
#define USERLIST_H

#include "../../cards/card_info_picture_art_crop_widget.h"
#include "user_avatar_provider.h"
#include "user_card_art_provider.h"
#include "user_info_popup.h"
#include "user_list_manager.h"
#include "user_list_painter.h"

#include <QComboBox>
#include <QDialog>
#include <QGroupBox>
#include <QQueue>
#include <QStyledItemDelegate>
#include <QTextEdit>
#include <QTreeWidgetItem>
#include <libcockatrice/network/server/remote/user_level.h>
#include <libcockatrice/protocol/pb/moderator_commands.pb.h>

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

class BanDialog : public QDialog
{
    Q_OBJECT
private:
    QLabel *daysLabel, *hoursLabel, *minutesLabel;
    QCheckBox *nameBanCheckBox, *ipBanCheckBox, *idBanCheckBox, *deleteMessages;
    QLineEdit *nameBanEdit, *ipBanEdit, *idBanEdit;
    QSpinBox *daysEdit, *hoursEdit, *minutesEdit;
    QRadioButton *permanentRadio, *temporaryRadio;
    QPlainTextEdit *reasonEdit, *visibleReasonEdit;
private slots:
    void okClicked();
    void enableTemporaryEdits(bool enabled);

public:
    explicit BanDialog(const ServerInfo_User &info, QWidget *parent = nullptr);
    [[nodiscard]] QString getBanName() const;
    [[nodiscard]] QString getBanIP() const;
    [[nodiscard]] QString getBanId() const;
    [[nodiscard]] int getMinutes() const;
    [[nodiscard]] QString getReason() const;
    [[nodiscard]] QString getVisibleReason() const;
    [[nodiscard]] int getDeleteMessages() const;
};

class WarningDialog : public QDialog
{
    Q_OBJECT
private:
    QLabel *descriptionLabel;
    QLineEdit *nameWarning;
    QComboBox *warningOption;
    QLineEdit *warnClientID;
    QCheckBox *deleteMessages;
private slots:
    void okClicked();

public:
    WarningDialog(const QString userName, const QString clientID, QWidget *parent = nullptr);
    [[nodiscard]] QString getName() const;
    [[nodiscard]] QString getWarnID() const;
    [[nodiscard]] QString getReason() const;
    [[nodiscard]] int getDeleteMessages() const;
    void addWarningOption(const QString warning);
};

class AdminNotesDialog : public QDialog
{
    Q_OBJECT

private:
    QString userName;
    QPlainTextEdit *notes;

public:
    explicit AdminNotesDialog(const QString &_userName, const QString &_notes, QWidget *_parent = nullptr);
    [[nodiscard]] QString getName() const
    {
        return userName;
    }
    [[nodiscard]] QString getNotes() const;
};

class UserListItemDelegate : public QStyledItemDelegate
{
    const QMap<QString, QPixmap> *avatarCache;
    const QMap<QString, QPixmap> *cardArtCache;
    const QMap<QString, CardArtParams> *cardArtParamsMap;

public:
    explicit UserListItemDelegate(QObject *const parent,
                                  const QMap<QString, QPixmap> *avatarCache,
                                  const QMap<QString, QPixmap> *cardArtCache,
                                  const QMap<QString, CardArtParams> *cardArtParamsMap);
    bool editorEvent(QEvent *event,
                     QAbstractItemModel *model,
                     const QStyleOptionViewItem &option,
                     const QModelIndex &index) override;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

class UserListTWI : public QTreeWidgetItem
{
private:
    ServerInfo_User userInfo;

public:
    explicit UserListTWI(const ServerInfo_User &_userInfo);
    const ServerInfo_User &getUserInfo() const
    {
        return userInfo;
    }
    void setUserInfo(const ServerInfo_User &_userInfo);
    void setOnline(bool online);
    bool operator<(const QTreeWidgetItem &other) const override;
};

class UserListWidget : public QGroupBox
{
    Q_OBJECT
public:
    enum UserListType
    {
        AllUsersList,
        RoomList,
        BuddyList,
        IgnoreList
    };

private:
    UserListManager *manager = nullptr;
    UserAvatarProvider *avatarProvider = nullptr;
    UserCardArtProvider *cardArtProvider = nullptr;
    QMap<QString, CardArtParams> cardArtParamsMap;
    // ── Hover popup ───────────────────────────────────────────────────────────
    UserInfoPopup *m_userInfoPopup = nullptr;
    QTimer *m_showPopupTimer = nullptr;
    QTimer *m_hidePopupTimer = nullptr;
    QString m_hoveredUser;
    bool m_popupPinned = false;

    void showPopupForUser(const QString &userName);
    void hidePopup(bool immediate = false);
    void positionPopup(const QString &userName);
    void connectPopupSignals();

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
    void refreshPopupButtons(const QString &userName);
private slots:
    void userClicked(QTreeWidgetItem *item, int column);
signals:
    void openMessageDialog(const QString &userName, bool focus);
    void addBuddy(const QString &userName);
    void removeBuddy(const QString &userName);
    void addIgnore(const QString &userName);
    void removeIgnore(const QString &userName);
    void joinGameRequested(int gameId, int roomId, bool asSpectator);

public:
    UserListWidget(TabSupervisor *_tabSupervisor,
                   AbstractClient *_client,
                   UserListType _type,
                   QWidget *parent = nullptr);
    void bind(UserListManager *mgr);
    void applyDisplayMode();
    bool eventFilter(QObject *obj, QEvent *event) override;
    void retranslateUi();
    void rebuild();
    void processUserInfo(const ServerInfo_User &user, bool online);
    bool deleteUser(const QString &userName);
    void setUserOnline(const QString &userName, bool online);
    [[nodiscard]] const QMap<QString, UserListTWI *> &getUsers() const
    {
        return users;
    }
    void showContextMenu(const QPoint &pos, const QModelIndex &index);
    void sortItems();

protected:
    void hideEvent(QHideEvent *e) override;
};

#endif
