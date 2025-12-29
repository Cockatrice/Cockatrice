/**
 * @file tab_supervisor.h
 * @ingroup Core
 * @ingroup Tabs
 * @brief TODO: Document this.
 */

#ifndef TAB_SUPERVISOR_H
#define TAB_SUPERVISOR_H

#include "../../deck_loader/deck_loader.h"
#include "../interface/widgets/server/user/user_list_proxy.h"
#include "abstract_tab_deck_editor.h"
#include "api/archidekt/tab_archidekt.h"
#include "api/edhrec/tab_edhrec.h"
#include "api/edhrec/tab_edhrec_main.h"
#include "tab_visual_database_display.h"
#include "visual_deck_editor/tab_deck_editor_visual.h"
#include "visual_deck_storage/tab_deck_storage_visual.h"

#include <QAbstractButton>
#include <QLoggingCategory>
#include <QMap>
#include <QProxyStyle>
#include <QTabWidget>

inline Q_LOGGING_CATEGORY(TabSupervisorLog, "tab_supervisor");

class UserListManager;
class QMenu;
class AbstractClient;
class Tab;
class TabServer;
class TabRoom;
class TabHome;
class TabGame;
class TabDeckStorage;
class TabReplays;
class TabAdmin;
class TabMessage;
class TabAccount;
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
    QRect subElementRect(SubElement, const QStyleOption *, const QWidget *) const override;
};

class CloseButton : public QAbstractButton
{
    Q_OBJECT
public:
    explicit CloseButton(QWidget *parent = nullptr);
    [[nodiscard]] QSize sizeHint() const override;
    [[nodiscard]] QSize minimumSizeHint() const override
    {
        return sizeHint();
    }

protected:
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    void enterEvent(QEnterEvent *event) override;
#else
    void enterEvent(QEvent *event) override;
#endif
    void leaveEvent(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
};

class TabSupervisor : public QTabWidget
{
    Q_OBJECT

public:
    enum DeckEditorType
    {
        ClassicDeckEditor,
        VisualDeckEditor
    };

private:
    ServerInfo_User *userInfo;
    AbstractClient *client;
    UserListManager *userListManager;
    QList<AbstractClient *> localClients;
    QMenu *tabsMenu;
    TabHome *tabHome;
    TabDeckStorageVisual *tabVisualDeckStorage;
    TabServer *tabServer;
    TabAccount *tabAccount;
    TabDeckStorage *tabDeckStorage;
    TabReplays *tabReplays;
    TabAdmin *tabAdmin;
    TabLog *tabLog;
    QMap<int, TabRoom *> roomTabs;
    QMap<int, TabGame *> gameTabs;
    QList<TabGame *> replayTabs;
    QMap<QString, TabMessage *> messageTabs;
    QList<AbstractTabDeckEditor *> deckEditorTabs;
    bool isLocalGame;

    QAction *aTabHome, *aTabDeckEditor, *aTabVisualDeckEditor, *aTabEdhRec, *aTabArchidekt, *aTabVisualDeckStorage,
        *aTabVisualDatabaseDisplay, *aTabServer, *aTabAccount, *aTabDeckStorage, *aTabReplays, *aTabAdmin, *aTabLog;

    int myAddTab(Tab *tab, QAction *manager = nullptr);
    void addCloseButtonToTab(Tab *tab, int tabIndex, QAction *manager);
    static QString sanitizeTabName(QString dirty);
    static QString sanitizeHtml(QString dirty);
    void resetTabsMenu();

public:
    explicit TabSupervisor(AbstractClient *_client, QMenu *tabsMenu, QWidget *parent = nullptr);
    ~TabSupervisor() override;
    void retranslateUi();
    void initStartupTabs();
    void start(const ServerInfo_User &userInfo);
    void startLocal(const QList<AbstractClient *> &_clients);
    void stop();
    [[nodiscard]] bool getIsLocalGame() const
    {
        return isLocalGame;
    }
    [[nodiscard]] int getGameCount() const
    {
        return gameTabs.size();
    }
    [[nodiscard]] TabAccount *getTabAccount() const
    {
        return tabAccount;
    }
    [[nodiscard]] ServerInfo_User *getUserInfo() const
    {
        return userInfo;
    }
    [[nodiscard]] AbstractClient *getClient() const;
    [[nodiscard]] const UserListManager *getUserListManager() const
    {
        return userListManager;
    }
    [[nodiscard]] const QMap<int, TabRoom *> &getRoomTabs() const
    {
        return roomTabs;
    }
    [[nodiscard]] QList<AbstractTabDeckEditor *> getDeckEditorTabs() const
    {
        return deckEditorTabs;
    }
    [[nodiscard]] bool getAdminLocked() const;
    void closeEvent(QCloseEvent *event) override;
    bool switchToGameTabIfAlreadyExists(const int gameId);
    static void actShowPopup(const QString &message);
signals:
    void setMenu(const QList<QMenu *> &newMenuList = QList<QMenu *>());
    void localGameEnded();
    void adminLockChanged(bool lock);
    void showWindowIfHidden();

public slots:
    void openDeckInNewTab(const LoadedDeck &deckToOpen);
    TabDeckEditor *addDeckEditorTab(const LoadedDeck &deckToOpen);
    TabDeckEditorVisual *addVisualDeckEditorTab(const LoadedDeck &deckToOpen);
    TabVisualDatabaseDisplay *addVisualDatabaseDisplayTab();
    TabEdhRecMain *addEdhrecMainTab();
    TabArchidekt *addArchidektTab();
    TabEdhRec *addEdhrecTab(const CardInfoPtr &cardToQuery, bool isCommander = false);
    void openReplay(GameReplay *replay);
    void switchToFirstAvailableNetworkTab();
    void maximizeMainWindow();
    void actTabVisualDeckStorage(bool checked);
    void actTabReplays(bool checked);
private slots:
    void refreshShortcuts();

    void actTabHome(bool checked);
    void actTabServer(bool checked);
    void actTabAccount(bool checked);
    void actTabDeckStorage(bool checked);
    void actTabAdmin(bool checked);
    void actTabLog(bool checked);

    void openTabVisualDeckStorage();
    void openTabHome();
    void openTabServer();
    void openTabAccount();
    void openTabDeckStorage();
    void openTabReplays();
    void openTabAdmin();
    void openTabLog();

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
    void deckEditorClosed(AbstractTabDeckEditor *tab);
    void tabUserEvent(bool globalEvent);
    void updateTabText(Tab *tab, const QString &newTabText);
    void processRoomEvent(const RoomEvent &event);
    void processGameEventContainer(const GameEventContainer &cont);
    void processUserMessageEvent(const Event_UserMessage &event);
    void processNotifyUserEvent(const Event_NotifyUser &event);
};

#endif
