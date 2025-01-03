#ifndef TAB_SUPERVISOR_H
#define TAB_SUPERVISOR_H

#include "../../deck/deck_loader.h"
#include "../../server/chat_view/user_list_proxy.h"
#include "visual_deck_storage/tab_deck_storage_visual.h"

#include <QAbstractButton>
#include <QCommonStyle>
#include <QMap>
#include <QProxyStyle>
#include <QTabWidget>

class QMenu;
class AbstractClient;
class Tab;
class TabServer;
class TabRoom;
class TabGame;
class TabDeckStorage;
class TabReplays;
class TabAdmin;
class TabMessage;
class TabUserLists;
class TabDeckEditor;
class TabLog;
class RoomEvent;
class GameEventContainer;
class Event_GameJoined;
class Event_UserMessage;
class Event_NotifyUser;
class ServerInfo_Room;
class ServerInfo_User;
class GameReplay;
class DeckList;

class MacOSTabFixStyle : public QProxyStyle
{
    Q_OBJECT
public:
    QRect subElementRect(SubElement, const QStyleOption *, const QWidget *) const;
};

class CloseButton : public QAbstractButton
{
    Q_OBJECT
public:
    CloseButton(QWidget *parent = nullptr);
    QSize sizeHint() const;
    inline QSize minimumSizeHint() const
    {
        return sizeHint();
    }

protected:
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    void enterEvent(QEnterEvent *event);
#else
    void enterEvent(QEvent *event);
#endif
    void leaveEvent(QEvent *event);
    void paintEvent(QPaintEvent *event);
};

class TabSupervisor : public QTabWidget, public UserlistProxy
{
    Q_OBJECT
private:
    ServerInfo_User *userInfo;
    AbstractClient *client;
    QList<AbstractClient *> localClients;
    TabServer *tabServer;
    TabUserLists *tabUserLists;
    TabDeckStorage *tabDeckStorage;
    TabReplays *tabReplays;
    TabAdmin *tabAdmin;
    TabLog *tabLog;
    QMap<int, TabRoom *> roomTabs;
    QMap<int, TabGame *> gameTabs;
    QList<TabGame *> replayTabs;
    QMap<QString, TabMessage *> messageTabs;
    QList<TabDeckEditor *> deckEditorTabs;
    int myAddTab(Tab *tab);
    void addCloseButtonToTab(Tab *tab, int tabIndex);
    QString sanitizeTabName(QString dirty) const;
    QString sanitizeHtml(QString dirty) const;
    bool isLocalGame;

public:
    TabSupervisor(AbstractClient *_client, QWidget *parent = nullptr);
    ~TabSupervisor();
    void retranslateUi();
    void start(const ServerInfo_User &userInfo);
    void startLocal(const QList<AbstractClient *> &_clients);
    void stop();
    bool getIsLocalGame() const
    {
        return isLocalGame;
    }
    int getGameCount() const
    {
        return gameTabs.size();
    }
    TabUserLists *getUserListsTab() const
    {
        return tabUserLists;
    }
    ServerInfo_User *getUserInfo() const
    {
        return userInfo;
    }
    AbstractClient *getClient() const;
    const QMap<int, TabRoom *> &getRoomTabs() const
    {
        return roomTabs;
    }
    bool getAdminLocked() const;
    bool closeRequest();
    bool isOwnUserRegistered() const;
    QString getOwnUsername() const;
    bool isUserBuddy(const QString &userName) const;
    bool isUserIgnored(const QString &userName) const;
    const ServerInfo_User *getOnlineUser(const QString &userName) const;
    bool switchToGameTabIfAlreadyExists(const int gameId);
    void actShowPopup(const QString &message);
signals:
    void setMenu(const QList<QMenu *> &newMenuList = QList<QMenu *>());
    void localGameEnded();
    void adminLockChanged(bool lock);
    void showWindowIfHidden();

public slots:
    TabDeckEditor *addDeckEditorTab(const DeckLoader *deckToOpen);
    TabDeckStorageVisual *addVisualDeckStorageTab();
    void openReplay(GameReplay *replay);
    void maximizeMainWindow();
private slots:
    void closeButtonPressed();
    void updateCurrent(int index);
    void updatePingTime(int value, int max);
    void gameJoined(const Event_GameJoined &event);
    void localGameJoined(const Event_GameJoined &event);
    void gameLeft(TabGame *tab);
    void addRoomTab(const ServerInfo_Room &info, bool setCurrent);
    void roomLeft(TabRoom *tab);
    TabMessage *addMessageTab(const QString &userName, bool focus);
    void replayLeft(TabGame *tab);
    void processUserLeft(const QString &userName);
    void processUserJoined(const ServerInfo_User &userInfo);
    void talkLeft(TabMessage *tab);
    void deckEditorClosed(TabDeckEditor *tab);
    void tabUserEvent(bool globalEvent);
    void updateTabText(Tab *tab, const QString &newTabText);
    void processRoomEvent(const RoomEvent &event);
    void processGameEventContainer(const GameEventContainer &cont);
    void processUserMessageEvent(const Event_UserMessage &event);
    void processNotifyUserEvent(const Event_NotifyUser &event);
};

#endif