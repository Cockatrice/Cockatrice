#include <QApplication>
#include "tab_supervisor.h"
#include "abstractclient.h"
#include "tab_server.h"
#include "tab_room.h"
#include "tab_game.h"
#include "tab_deck_storage.h"
#include "tab_replays.h"
#include "tab_admin.h"
#include "tab_message.h"
#include "tab_userlists.h"
#include "tab_deck_editor.h"
#include "pixmapgenerator.h"
#include "userlist.h"
#include "settingscache.h"
#include <QDebug>
#include <QPainter>

#include "pb/room_commands.pb.h"
#include "pb/room_event.pb.h"
#include "pb/game_event_container.pb.h"
#include "pb/event_user_message.pb.h"
#include "pb/event_game_joined.pb.h"
#include "pb/serverinfo_user.pb.h"
#include "pb/serverinfo_room.pb.h"

CloseButton::CloseButton(QWidget *parent)
    : QAbstractButton(parent)
{
    setFocusPolicy(Qt::NoFocus);
    setCursor(Qt::ArrowCursor);
    resize(sizeHint());
}

QSize CloseButton::sizeHint() const
{
    ensurePolished();
    int width = style()->pixelMetric(QStyle::PM_TabCloseIndicatorWidth, 0, this);
    int height = style()->pixelMetric(QStyle::PM_TabCloseIndicatorHeight, 0, this);
    return QSize(width, height);
}

void CloseButton::enterEvent(QEvent *event)
{
    update();
    QAbstractButton::enterEvent(event);
}

void CloseButton::leaveEvent(QEvent *event)
{
    update();
    QAbstractButton::leaveEvent(event);
}

void CloseButton::paintEvent(QPaintEvent * /*event*/)
{
    QPainter p(this);
    QStyleOption opt;
    opt.init(this);
    opt.state |= QStyle::State_AutoRaise;
    if (isEnabled() && underMouse() && !isChecked() && !isDown())
        opt.state |= QStyle::State_Raised;
    if (isChecked())
        opt.state |= QStyle::State_On;
    if (isDown())
        opt.state |= QStyle::State_Sunken;
    
    if (const QTabBar *tb = qobject_cast<const QTabBar *>(parent())) {
        int index = tb->currentIndex();
        QTabBar::ButtonPosition position = (QTabBar::ButtonPosition) style()->styleHint(QStyle::SH_TabBar_CloseButtonPosition, 0, tb);
        if (tb->tabButton(index, position) == this)
            opt.state |= QStyle::State_Selected;
    }
    
    style()->drawPrimitive(QStyle::PE_IndicatorTabClose, &opt, &p, this);
}

TabSupervisor::TabSupervisor(AbstractClient *_client, QWidget *parent)
    : QTabWidget(parent), userInfo(0), client(_client), tabUserLists(0), tabServer(0), tabDeckStorage(0), tabAdmin(0), tabReplays(0)
{
    tabChangedIcon = new QIcon(":/resources/icon_tab_changed.svg");
    setElideMode(Qt::ElideRight);
    setMovable(true);
    setIconSize(QSize(15, 15));
    connect(this, SIGNAL(currentChanged(int)), this, SLOT(updateCurrent(int)));

    connect(client, SIGNAL(roomEventReceived(const RoomEvent &)), this, SLOT(processRoomEvent(const RoomEvent &)));
    connect(client, SIGNAL(gameEventContainerReceived(const GameEventContainer &)), this, SLOT(processGameEventContainer(const GameEventContainer &)));
    connect(client, SIGNAL(gameJoinedEventReceived(const Event_GameJoined &)), this, SLOT(gameJoined(const Event_GameJoined &)));
    connect(client, SIGNAL(userMessageEventReceived(const Event_UserMessage &)), this, SLOT(processUserMessageEvent(const Event_UserMessage &)));
    connect(client, SIGNAL(maxPingTime(int, int)), this, SLOT(updatePingTime(int, int)));
    
    retranslateUi();
}

TabSupervisor::~TabSupervisor()
{
    stop();
    delete tabChangedIcon;
}

void TabSupervisor::retranslateUi()
{
    QList<Tab *> tabs;
    tabs.append(tabServer);
    tabs.append(tabReplays);
    tabs.append(tabDeckStorage);
    tabs.append(tabAdmin);
    tabs.append(tabUserLists);
    QMapIterator<int, TabRoom *> roomIterator(roomTabs);
    while (roomIterator.hasNext())
        tabs.append(roomIterator.next().value());
    QMapIterator<int, TabGame *> gameIterator(gameTabs);
    while (gameIterator.hasNext())
        tabs.append(gameIterator.next().value());
    QListIterator<TabGame *> replayIterator(replayTabs);
    while (replayIterator.hasNext())
        tabs.append(replayIterator.next());
    QListIterator<TabDeckEditor *> deckEditorIterator(deckEditorTabs);
    while (deckEditorIterator.hasNext())
        tabs.append(deckEditorIterator.next());
    QMapIterator<QString, TabMessage *> messageIterator(messageTabs);
    while (messageIterator.hasNext())
        tabs.append(messageIterator.next().value());
    
    for (int i = 0; i < tabs.size(); ++i)
        if (tabs[i]) {
            setTabText(indexOf(tabs[i]), tabs[i]->getTabText());
            tabs[i]->retranslateUi();
        }
}

AbstractClient *TabSupervisor::getClient() const
{
    return localClients.isEmpty() ? client : localClients.first();
}

int TabSupervisor::myAddTab(Tab *tab)
{
    connect(tab, SIGNAL(userEvent(bool)), this, SLOT(tabUserEvent(bool)));
    connect(tab, SIGNAL(tabTextChanged(Tab *, QString)), this, SLOT(updateTabText(Tab *, QString)));
    return addTab(tab, tab->getTabText());
}

void TabSupervisor::start(const ServerInfo_User &_userInfo)
{
    userInfo = new ServerInfo_User(_userInfo);
    
    tabServer = new TabServer(this, client);
    connect(tabServer, SIGNAL(roomJoined(const ServerInfo_Room &, bool)), this, SLOT(addRoomTab(const ServerInfo_Room &, bool)));
    myAddTab(tabServer);
    
    tabUserLists = new TabUserLists(this, client, *userInfo);
    connect(tabUserLists, SIGNAL(openMessageDialog(const QString &, bool)), this, SLOT(addMessageTab(const QString &, bool)));
    connect(tabUserLists, SIGNAL(userJoined(ServerInfo_User)), this, SLOT(processUserJoined(ServerInfo_User)));
    connect(tabUserLists, SIGNAL(userLeft(const QString &)), this, SLOT(processUserLeft(const QString &)));
    myAddTab(tabUserLists);
    
    updatePingTime(0, -1);
    
    if (userInfo->user_level() & ServerInfo_User::IsRegistered) {
        tabDeckStorage = new TabDeckStorage(this, client);
        connect(tabDeckStorage, SIGNAL(openDeckEditor(const DeckLoader *)), this, SLOT(addDeckEditorTab(const DeckLoader *)));
        myAddTab(tabDeckStorage);
        
        tabReplays = new TabReplays(this, client);
        connect(tabReplays, SIGNAL(openReplay(GameReplay *)), this, SLOT(openReplay(GameReplay *)));
        myAddTab(tabReplays);
    } else {
        tabDeckStorage = 0;
        tabReplays = 0;
    }
    
    if (userInfo->user_level() & ServerInfo_User::IsModerator) {
        tabAdmin = new TabAdmin(this, client, (userInfo->user_level() & ServerInfo_User::IsAdmin));
        connect(tabAdmin, SIGNAL(adminLockChanged(bool)), this, SIGNAL(adminLockChanged(bool)));
        myAddTab(tabAdmin);
    } else
        tabAdmin = 0;

    retranslateUi();
}

void TabSupervisor::startLocal(const QList<AbstractClient *> &_clients)
{
    tabUserLists = 0;
    tabDeckStorage = 0;
    tabReplays = 0;
    tabAdmin = 0;
    userInfo = new ServerInfo_User;
    localClients = _clients;
    for (int i = 0; i < localClients.size(); ++i)
        connect(localClients[i], SIGNAL(gameEventContainerReceived(const GameEventContainer &)), this, SLOT(processGameEventContainer(const GameEventContainer &)));
    connect(localClients.first(), SIGNAL(gameJoinedEventReceived(const Event_GameJoined &)), this, SLOT(localGameJoined(const Event_GameJoined &)));
}

void TabSupervisor::stop()
{
    if ((!client) && localClients.isEmpty())
        return;
    
    if (!localClients.isEmpty()) {
        for (int i = 0; i < localClients.size(); ++i)
            localClients[i]->deleteLater();
        localClients.clear();
        
        emit localGameEnded();
    } else {
        if (tabUserLists)
            tabUserLists->deleteLater();
        if (tabServer)
            tabServer->deleteLater();
        if (tabDeckStorage)
            tabDeckStorage->deleteLater();
        if (tabReplays)
            tabReplays->deleteLater();
        if (tabAdmin)
            tabAdmin->deleteLater();
    }
    tabUserLists = 0;
    tabServer = 0;
    tabDeckStorage = 0;
    tabReplays = 0;
    tabAdmin = 0;
    
    QMapIterator<int, TabRoom *> roomIterator(roomTabs);
    while (roomIterator.hasNext())
        roomIterator.next().value()->deleteLater();
    roomTabs.clear();

    QMapIterator<int, TabGame *> gameIterator(gameTabs);
    while (gameIterator.hasNext())
        gameIterator.next().value()->deleteLater();
    gameTabs.clear();

    QListIterator<TabGame *> replayIterator(replayTabs);
    while (replayIterator.hasNext())
        replayIterator.next()->deleteLater();
    replayTabs.clear();

    QMapIterator<QString, TabMessage *> messageIterator(messageTabs);
    while (messageIterator.hasNext())
        messageIterator.next().value()->deleteLater();
    messageTabs.clear();
    
    delete userInfo;
    userInfo = 0;
}

void TabSupervisor::updatePingTime(int value, int max)
{
    if (!tabServer)
        return;
    if (tabServer->getContentsChanged())
        return;
    
    setTabIcon(indexOf(tabServer), QIcon(PingPixmapGenerator::generatePixmap(15, value, max)));
}

void TabSupervisor::closeButtonPressed()
{
    Tab *tab = static_cast<Tab *>(static_cast<CloseButton *>(sender())->property("tab").value<QObject *>());
    tab->closeRequest();
}

void TabSupervisor::addCloseButtonToTab(Tab *tab, int tabIndex)
{
    QTabBar::ButtonPosition closeSide = (QTabBar::ButtonPosition) tabBar()->style()->styleHint(QStyle::SH_TabBar_CloseButtonPosition, 0, tabBar());
    CloseButton *closeButton = new CloseButton;
    connect(closeButton, SIGNAL(clicked()), this, SLOT(closeButtonPressed()));
    closeButton->setProperty("tab", qVariantFromValue((QObject *) tab));
    tabBar()->setTabButton(tabIndex, closeSide, closeButton);
}

void TabSupervisor::gameJoined(const Event_GameJoined &event)
{
    QMap<int, QString> roomGameTypes;
    TabRoom *room = roomTabs.value(event.game_info().room_id());
    if (room)
        roomGameTypes = room->getGameTypes();
    else
        for (int i = 0; i < event.game_types_size(); ++i)
            roomGameTypes.insert(event.game_types(i).game_type_id(), QString::fromStdString(event.game_types(i).description()));
    
    TabGame *tab = new TabGame(this, QList<AbstractClient *>() << client, event, roomGameTypes);
    connect(tab, SIGNAL(gameClosing(TabGame *)), this, SLOT(gameLeft(TabGame *)));
    connect(tab, SIGNAL(openMessageDialog(const QString &, bool)), this, SLOT(addMessageTab(const QString &, bool)));
    connect(tab, SIGNAL(openDeckEditor(const DeckLoader *)), this, SLOT(addDeckEditorTab(const DeckLoader *)));
    int tabIndex = myAddTab(tab);
    addCloseButtonToTab(tab, tabIndex);
    gameTabs.insert(event.game_info().game_id(), tab);
    setCurrentWidget(tab);
}

void TabSupervisor::localGameJoined(const Event_GameJoined &event)
{
    TabGame *tab = new TabGame(this, localClients, event, QMap<int, QString>());
    connect(tab, SIGNAL(gameClosing(TabGame *)), this, SLOT(gameLeft(TabGame *)));
    connect(tab, SIGNAL(openDeckEditor(const DeckLoader *)), this, SLOT(addDeckEditorTab(const DeckLoader *)));
    int tabIndex = myAddTab(tab);
    addCloseButtonToTab(tab, tabIndex);
    gameTabs.insert(event.game_info().game_id(), tab);
    setCurrentWidget(tab);
    
    for (int i = 1; i < localClients.size(); ++i) {
        Command_JoinGame cmd;
        cmd.set_game_id(event.game_info().game_id());
        localClients[i]->sendCommand(localClients[i]->prepareRoomCommand(cmd, 0));
    }
}

void TabSupervisor::gameLeft(TabGame *tab)
{
    if (tab == currentWidget())
        emit setMenu();

    gameTabs.remove(tab->getGameId());
    removeTab(indexOf(tab));
    
    if (!localClients.isEmpty())
        stop();
}

void TabSupervisor::addRoomTab(const ServerInfo_Room &info, bool setCurrent)
{
    TabRoom *tab = new TabRoom(this, client, userInfo, info);
    connect(tab, SIGNAL(roomClosing(TabRoom *)), this, SLOT(roomLeft(TabRoom *)));
    connect(tab, SIGNAL(openMessageDialog(const QString &, bool)), this, SLOT(addMessageTab(const QString &, bool)));
    int tabIndex = myAddTab(tab);
    addCloseButtonToTab(tab, tabIndex);
    roomTabs.insert(info.room_id(), tab);
    if (setCurrent)
        setCurrentWidget(tab);
}

void TabSupervisor::roomLeft(TabRoom *tab)
{
    if (tab == currentWidget())
        emit setMenu();
    
    roomTabs.remove(tab->getRoomId());
    removeTab(indexOf(tab));
}

void TabSupervisor::openReplay(GameReplay *replay)
{
    TabGame *replayTab = new TabGame(this, replay);
    connect(replayTab, SIGNAL(gameClosing(TabGame *)), this, SLOT(replayLeft(TabGame *)));
    int tabIndex = myAddTab(replayTab);
    addCloseButtonToTab(replayTab, tabIndex);
    replayTabs.append(replayTab);
    setCurrentWidget(replayTab);
}

void TabSupervisor::replayLeft(TabGame *tab)
{
    if (tab == currentWidget())
        emit setMenu();
    
    replayTabs.removeAt(replayTabs.indexOf(tab));
}

TabMessage *TabSupervisor::addMessageTab(const QString &receiverName, bool focus)
{
    if (receiverName == QString::fromStdString(userInfo->name()))
        return 0;
    
    ServerInfo_User otherUser;
    UserListTWI *twi = tabUserLists->getAllUsersList()->getUsers().value(receiverName);
    if (twi)
        otherUser = twi->getUserInfo();
    else
        otherUser.set_name(receiverName.toStdString());
    TabMessage *tab = new TabMessage(this, client, *userInfo, otherUser);
    connect(tab, SIGNAL(talkClosing(TabMessage *)), this, SLOT(talkLeft(TabMessage *)));
    int tabIndex = myAddTab(tab);
    addCloseButtonToTab(tab, tabIndex);
    messageTabs.insert(receiverName, tab);
    if (focus)
        setCurrentWidget(tab);
    return tab;
}

void TabSupervisor::talkLeft(TabMessage *tab)
{
    if (tab == currentWidget())
        emit setMenu();

    messageTabs.remove(tab->getUserName());
    removeTab(indexOf(tab));
}

TabDeckEditor *TabSupervisor::addDeckEditorTab(const DeckLoader *deckToOpen)
{
    TabDeckEditor *tab = new TabDeckEditor(this);
    if (deckToOpen)
        tab->setDeck(new DeckLoader(*deckToOpen));
    connect(tab, SIGNAL(deckEditorClosing(TabDeckEditor *)), this, SLOT(deckEditorClosed(TabDeckEditor *)));
    int tabIndex = myAddTab(tab);
    addCloseButtonToTab(tab, tabIndex);
    deckEditorTabs.append(tab);
    setCurrentWidget(tab);
    return tab;
}

void TabSupervisor::deckEditorClosed(TabDeckEditor *tab)
{
    if (tab == currentWidget())
        emit setMenu();
    
    deckEditorTabs.removeAt(deckEditorTabs.indexOf(tab));
    removeTab(indexOf(tab));
}

void TabSupervisor::tabUserEvent(bool globalEvent)
{
    Tab *tab = static_cast<Tab *>(sender());
    if (tab != currentWidget()) {
        tab->setContentsChanged(true);
        setTabIcon(indexOf(tab), *tabChangedIcon);
    }
    if (globalEvent && settingsCache->getNotificationsEnabled())
        QApplication::alert(this);
}

void TabSupervisor::updateTabText(Tab *tab, const QString &newTabText)
{
    setTabText(indexOf(tab), newTabText);
}

void TabSupervisor::processRoomEvent(const RoomEvent &event)
{
    TabRoom *tab = roomTabs.value(event.room_id(), 0);
    if (tab)
        tab->processRoomEvent(event);
}

void TabSupervisor::processGameEventContainer(const GameEventContainer &cont)
{
    TabGame *tab = gameTabs.value(cont.game_id());
    if (tab)
        tab->processGameEventContainer(cont, qobject_cast<AbstractClient *>(sender()));
    else
        qDebug() << "gameEvent: invalid gameId";
}

void TabSupervisor::processUserMessageEvent(const Event_UserMessage &event)
{
    TabMessage *tab = messageTabs.value(QString::fromStdString(event.sender_name()));
    if (!tab)
        tab = messageTabs.value(QString::fromStdString(event.receiver_name()));
    if (!tab)
        tab = addMessageTab(QString::fromStdString(event.sender_name()), false);
    if (!tab)
        return;
    tab->processUserMessageEvent(event);
}

void TabSupervisor::processUserLeft(const QString &userName)
{
    TabMessage *tab = messageTabs.value(userName);
    if (tab)
        tab->processUserLeft();
}

void TabSupervisor::processUserJoined(const ServerInfo_User &userInfo)
{
    TabMessage *tab = messageTabs.value(QString::fromStdString(userInfo.name()));
    if (tab)
        tab->processUserJoined(userInfo);
}

void TabSupervisor::updateCurrent(int index)
{
    if (index != -1) {
        Tab *tab = static_cast<Tab *>(widget(index));
        if (tab->getContentsChanged()) {
            setTabIcon(index, QIcon());
            tab->setContentsChanged(false);
        }
        emit setMenu(static_cast<Tab *>(widget(index))->getTabMenus());
    } else
        emit setMenu();
}

bool TabSupervisor::getAdminLocked() const
{
    if (!tabAdmin)
        return true;
    return tabAdmin->getLocked();
}
