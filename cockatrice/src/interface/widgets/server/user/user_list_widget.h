/**
 * @file user_list_widget.h
 * @ingroup Lobby
 */
//! \todo Document this file.

#ifndef USERLIST_H
#define USERLIST_H

#include "../../cards/card_info_picture_art_crop_widget.h"
#include "../../interface/widgets/server/game_link.h"
#include "user_avatar_provider.h"
#include "user_card_art_provider.h"
#include "user_info_popup.h"
#include "user_list_manager.h"
#include "user_list_painter.h"

#include <QComboBox>
#include <QDialog>
#include <QGroupBox>
#include <QQueue>
#include <QSet>
#include <QStyledItemDelegate>
#include <QTextEdit>
#include <QTreeWidgetItem>
#include <functional>
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
class UserListWidget;
class QShowEvent;

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
    QTreeWidget *tree;
    UserListWidget *owner;
    const QMap<QString, QPixmap> *avatarCache;
    const QMap<QString, QPixmap> *cardArtCache;
    const QMap<QString, CardArtParams> *cardArtParamsMap;

public:
    explicit UserListItemDelegate(UserListWidget *owner,
                                  QTreeWidget *tree,
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
    enum class Section
    {
        Buddy,
        Online,
        Ignore
    };

private:
    UserListManager *manager = nullptr;
    UserAvatarProvider *avatarProvider = nullptr;
    UserCardArtProvider *cardArtProvider = nullptr;
    QMap<QString, CardArtParams> cardArtParamsMap;
    // ── Hover popup ───────────────────────────────────────────────────────────
    UserInfoPopup *userInfoPopup = nullptr;
    QTimer *showPopupTimer = nullptr;
    QTimer *hidePopupTimer = nullptr;
    QString hoveredUser;
    bool popupPinned = false;
    bool bulkLoading = false;

    /**
     * Popup functions are anchored on the row, not the user name. In sectioned
     * mode a user can own several rows (online + buddy), and the popup must
     * follow the hovered/selected row rather than a lookup by name.
     */
    void showPopupForUser(UserListTWI *item);
    void hidePopup(bool immediate = false);
    void positionPopup(UserListTWI *item);
    void connectPopupSignals();
    /** True when @p widget is the tree, the popup or an open menu. */
    bool isPressInsideListUi(const QWidget *widget) const;
    void clearSelectionAndClosePopup();
    bool isItemNearViewport(const UserListTWI *item) const;
    void requestAvatarsForVisibleItems();

    // Sectioned mode (single tree with inline dividers)
    bool sectioned = false;
    QList<Section> sectionIds;
    QMap<Section, QTreeWidgetItem *> sectionItems;
    // One row per (section, user): a user that is online AND a buddy appears in
    // both the "Online" and the "Buddies" sections, so the same user can own
    // several rows, each hanging off its section's divider.
    QMap<Section, QMap<QString, UserListTWI *>> sectionUsers;
    QSet<Section> expandedSections;
    void createSectionItems();
    QTreeWidgetItem *createSectionItem(Section section);
    [[nodiscard]] QString sectionTitle(Section section) const;
    void updateSectionDivider(Section section);
    void handleSectionExpansion(QTreeWidgetItem *item, bool expanded);
    void setExpandedProgrammatically(QTreeWidgetItem *item, bool expanded);
    void handleOnlineChange(const ServerInfo_User &user);
    void handleOnlineChangeLeft(const QString &userName);
    void handleListAdd(Section section, const ServerInfo_User &user);
    void handleListRemove(Section section, const QString &userName);
    /** Creates or updates the row for @p user in @p section. */
    UserListTWI *ensureSectionMembership(Section section, const ServerInfo_User &user, bool online);
    /** Removes and deletes the row for @p userName in @p section. */
    bool dropSectionMembership(Section section, const QString &userName);
    /** Sorts, refilters and repaints after a sectioned mode mutation. */
    void finishSectionedMutation();
    void updateCardArtParams(const ServerInfo_User &user, const QString &userName);
    void processUserInfo(Section section, const ServerInfo_User &user, bool online);

    QMap<QString, UserListTWI *> users;
    TabSupervisor *tabSupervisor;
    AbstractClient *client;
    UserListType type;
    QTreeWidget *userTree = nullptr;
    UserListItemDelegate *itemDelegate;
    UserContextMenu *userContextMenu;
    int onlineCount;
    QString titleStr;
    QString filterText;
    bool showTitle = true;
    void updateCount();
    void applyFilter();
    void refreshPopupButtons(const QString &userName);
private slots:
    void userClicked(QTreeWidgetItem *item, int column);
    void refreshVisibleUserHeader(const QString &name);
signals:
    void openMessageDialog(const QString &userName, bool focus);
    void addBuddy(const QString &userName);
    void removeBuddy(const QString &userName);
    void addIgnore(const QString &userName);
    void removeIgnore(const QString &userName);
    void joinGameRequested(int gameId, int roomId, bool asSpectator);
    void sectionExpanded(Section section, bool expanded);

public:
    UserListWidget(TabSupervisor *_tabSupervisor,
                   AbstractClient *_client,
                   UserListType _type,
                   QWidget *parent = nullptr);
    ~UserListWidget() override;
    void bind(UserListManager *mgr);
    void applyDisplayMode();
    void beginBulkLoad();
    void endBulkLoad();
    bool eventFilter(QObject *obj, QEvent *event) override;
    void retranslateUi();
    void rebuild();
    void processUserInfo(const ServerInfo_User &user, bool online);
    bool deleteUser(const QString &userName);
    void setUserOnline(const QString &userName, bool online);
    void setFilterText(const QString &text);
    void setShowTitle(bool showTitle);
    void setSectioned(const QList<Section> &ids);
    void setSectionExpanded(Section section, bool expanded);
    [[nodiscard]] const QList<Section> &getSectionIds() const
    {
        return sectionIds;
    }
    [[nodiscard]] const QMap<QString, UserListTWI *> &getUsers() const
    {
        return users;
    }
    void showContextMenu(const QPoint &pos, const QModelIndex &index);
    void sortItems();
    void setGameInviteLinkProvider(std::function<QList<GameInviteOption>()> provider);

protected:
    void hideEvent(QHideEvent *e) override;
    void showEvent(QShowEvent *e) override;
};

#endif
