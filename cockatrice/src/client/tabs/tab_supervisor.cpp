#include "tab_supervisor.h"

#include "../../client/game_logic/abstract_client.h"
#include "../../main.h"
#include "../../server/user/user_list.h"
#include "../../settings/cache_settings.h"
#include "../ui/pixel_map_generator.h"
#include "pb/event_game_joined.pb.h"
#include "pb/event_notify_user.pb.h"
#include "pb/event_user_message.pb.h"
#include "pb/game_event_container.pb.h"
#include "pb/game_replay.pb.h"
#include "pb/moderator_commands.pb.h"
#include "pb/room_commands.pb.h"
#include "pb/room_event.pb.h"
#include "pb/serverinfo_room.pb.h"
#include "pb/serverinfo_user.pb.h"
#include "tab_account.h"
#include "tab_admin.h"
#include "tab_deck_editor.h"
#include "tab_deck_storage.h"
#include "tab_game.h"
#include "tab_logs.h"
#include "tab_message.h"
#include "tab_replays.h"
#include "tab_room.h"
#include "tab_server.h"
#include "visual_deck_storage/tab_deck_storage_visual.h"

#include <QApplication>
#include <QDebug>
#include <QMessageBox>
#include <QPainter>
#include <QSystemTrayIcon>

QRect MacOSTabFixStyle::subElementRect(SubElement element, const QStyleOption *option, const QWidget *widget) const
{
    if (element != SE_TabBarTabText) {
        return QProxyStyle::subElementRect(element, option, widget);
    }

    // Skip over QProxyStyle handling subElementRect,
    // This fixes an issue with Qt 5.10 on OSX where the labels for tabs with a button and an icon
    // get cut-off too early
    return QCommonStyle::subElementRect(element, option, widget);
}

CloseButton::CloseButton(QWidget *parent) : QAbstractButton(parent)
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

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
void CloseButton::enterEvent(QEnterEvent *event)
#else
void CloseButton::enterEvent(QEvent *event)
#endif
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
    opt.initFrom(this);
    opt.state |= QStyle::State_AutoRaise;
    if (isEnabled() && underMouse() && !isChecked() && !isDown())
        opt.state |= QStyle::State_Raised;
    if (isChecked())
        opt.state |= QStyle::State_On;
    if (isDown())
        opt.state |= QStyle::State_Sunken;

    if (const QTabBar *tb = qobject_cast<const QTabBar *>(parent())) {
        int index = tb->currentIndex();
        QTabBar::ButtonPosition position =
            (QTabBar::ButtonPosition)style()->styleHint(QStyle::SH_TabBar_CloseButtonPosition, 0, tb);
        if (tb->tabButton(index, position) == this)
            opt.state |= QStyle::State_Selected;
    }

    style()->drawPrimitive(QStyle::PE_IndicatorTabClose, &opt, &p, this);
}

TabSupervisor::TabSupervisor(AbstractClient *_client, QMenu *tabsMenu, QWidget *parent)
    : QTabWidget(parent), userInfo(0), client(_client), tabsMenu(tabsMenu), tabVisualDeckStorage(nullptr), tabServer(0),
      tabAccount(0), tabDeckStorage(0), tabReplays(0), tabAdmin(0), tabLog(0)
{
    setElideMode(Qt::ElideRight);
    setMovable(true);
    setIconSize(QSize(15, 15));

#if defined(Q_OS_MAC)
    // This is necessary to fix an issue on macOS with qt5.10,
    // where tabs with icons and buttons get drawn incorrectly
    tabBar()->setStyle(new MacOSTabFixStyle);
#endif

    // connect tab changes
    connect(this, &TabSupervisor::currentChanged, this, &TabSupervisor::updateCurrent);

    // connect client
    connect(client, &AbstractClient::roomEventReceived, this, &TabSupervisor::processRoomEvent);
    connect(client, &AbstractClient::gameEventContainerReceived, this, &TabSupervisor::processGameEventContainer);
    connect(client, &AbstractClient::gameJoinedEventReceived, this, &TabSupervisor::gameJoined);
    connect(client, &AbstractClient::userMessageEventReceived, this, &TabSupervisor::processUserMessageEvent);
    connect(client, &AbstractClient::maxPingTime, this, &TabSupervisor::updatePingTime);
    connect(client, &AbstractClient::notifyUserEventReceived, this, &TabSupervisor::processNotifyUserEvent);

    // create tabs menu actions
    aTabDeckEditor = new QAction(this);
    connect(aTabDeckEditor, &QAction::triggered, this, [this] { addDeckEditorTab(nullptr); });

    aTabVisualDeckStorage = new QAction(this);
    aTabVisualDeckStorage->setCheckable(true);
    connect(aTabVisualDeckStorage, &QAction::toggled, this, &TabSupervisor::actTabVisualDeckStorage);

    aTabServer = new QAction(this);
    aTabServer->setCheckable(true);
    connect(aTabServer, &QAction::toggled, this, &TabSupervisor::actTabServer);

    aTabAccount = new QAction(this);
    aTabAccount->setCheckable(true);
    connect(aTabAccount, &QAction::toggled, this, &TabSupervisor::actTabAccount);

    aTabDeckStorage = new QAction(this);
    aTabDeckStorage->setCheckable(true);
    connect(aTabDeckStorage, &QAction::toggled, this, &TabSupervisor::actTabDeckStorage);

    aTabReplays = new QAction(this);
    aTabReplays->setCheckable(true);
    connect(aTabReplays, &QAction::toggled, this, &TabSupervisor::actTabReplays);

    aTabAdmin = new QAction(this);
    aTabAdmin->setCheckable(true);
    connect(aTabAdmin, &QAction::toggled, this, &TabSupervisor::actTabAdmin);

    aTabLog = new QAction(this);
    aTabLog->setCheckable(true);
    connect(aTabLog, &QAction::toggled, this, &TabSupervisor::actTabLog);

    connect(&SettingsCache::instance().shortcuts(), &ShortcutsSettings::shortCutChanged, this,
            &TabSupervisor::refreshShortcuts);
    refreshShortcuts();

    resetTabsMenu();

    retranslateUi();

    // open always-available tabs on startup
    addDeckEditorTab(nullptr);

    aTabVisualDeckStorage->setChecked(SettingsCache::instance().getVisualDeckStorageShowOnLoad());
}

TabSupervisor::~TabSupervisor()
{
    stop();
}

void TabSupervisor::retranslateUi()
{
    // tab menu actions
    aTabDeckEditor->setText(tr("Deck Editor"));
    aTabVisualDeckStorage->setText(tr("&Visual Deck storage"));
    aTabServer->setText(tr("Server"));
    aTabAccount->setText(tr("Account"));
    aTabDeckStorage->setText(tr("Deck storage"));
    aTabReplays->setText(tr("Game replays"));
    aTabAdmin->setText(tr("Administration"));
    aTabLog->setText(tr("Logs"));

    // tabs
    QList<Tab *> tabs;
    tabs.append(tabServer);
    tabs.append(tabReplays);
    tabs.append(tabDeckStorage);
    tabs.append(tabAdmin);
    tabs.append(tabAccount);
    tabs.append(tabLog);
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
            int idx = indexOf(tabs[i]);
            QString tabText = tabs[i]->getTabText();
            setTabText(idx, sanitizeTabName(tabText));
            setTabToolTip(idx, sanitizeHtml(tabText));
            tabs[i]->retranslateUi();
        }
}

void TabSupervisor::refreshShortcuts()
{
    ShortcutsSettings &shortcuts = SettingsCache::instance().shortcuts();
    aTabDeckEditor->setShortcuts(shortcuts.getShortcut("MainWindow/aDeckEditor"));
}

bool TabSupervisor::closeRequest()
{
    if (getGameCount()) {
        if (QMessageBox::question(this, tr("Are you sure?"),
                                  tr("There are still open games. Are you sure you want to quit?"),
                                  QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::No) {
            return false;
        }
    }

    foreach (TabDeckEditor *tab, deckEditorTabs) {
        if (!tab->confirmClose())
            return false;
    }

    return true;
}

AbstractClient *TabSupervisor::getClient() const
{
    return localClients.isEmpty() ? client : localClients.first();
}

QString TabSupervisor::sanitizeTabName(QString dirty) const
{
    return dirty.replace("&", "&&");
}

QString TabSupervisor::sanitizeHtml(QString dirty) const
{
    return dirty.replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;").replace("\"", "&quot;");
}

int TabSupervisor::myAddTab(Tab *tab)
{
    connect(tab, &TabGame::userEvent, this, &TabSupervisor::tabUserEvent);
    connect(tab, &TabGame::tabTextChanged, this, &TabSupervisor::updateTabText);

    QString tabText = tab->getTabText();
    int idx = addTab(tab, sanitizeTabName(tabText));
    setTabToolTip(idx, sanitizeHtml(tabText));

    addCloseButtonToTab(tab, idx);

    return idx;
}

/**
 * Resets the tabs menu to the tabs that are always available
 */
void TabSupervisor::resetTabsMenu()
{
    tabsMenu->clear();
    tabsMenu->addAction(aTabDeckEditor);
    tabsMenu->addSeparator();
    tabsMenu->addAction(aTabVisualDeckStorage);
}

void TabSupervisor::start(const ServerInfo_User &_userInfo)
{
    isLocalGame = false;
    userInfo = new ServerInfo_User(_userInfo);

    resetTabsMenu();

    tabsMenu->addSeparator();
    tabsMenu->addAction(aTabServer);
    tabsMenu->addAction(aTabAccount);

    aTabServer->setChecked(true);
    aTabAccount->setChecked(true);

    updatePingTime(0, -1);

    if (userInfo->user_level() & ServerInfo_User::IsRegistered) {
        tabsMenu->addAction(aTabDeckStorage);
        tabsMenu->addAction(aTabReplays);

        aTabDeckStorage->setChecked(true);
        aTabReplays->setChecked(true);
    }

    if (userInfo->user_level() & ServerInfo_User::IsModerator) {
        tabsMenu->addSeparator();
        tabsMenu->addAction(aTabAdmin);
        tabsMenu->addAction(aTabLog);

        aTabAdmin->setChecked(true);
        aTabLog->setChecked(true);
    }

    retranslateUi();
}

void TabSupervisor::startLocal(const QList<AbstractClient *> &_clients)
{
    resetTabsMenu();

    tabAccount = 0;
    tabDeckStorage = 0;
    tabReplays = 0;
    tabAdmin = 0;
    tabLog = 0;
    isLocalGame = true;
    userInfo = new ServerInfo_User;
    localClients = _clients;
    for (int i = 0; i < localClients.size(); ++i)
        connect(localClients[i], &AbstractClient::gameEventContainerReceived, this,
                &TabSupervisor::processGameEventContainer);
    connect(localClients.first(), &AbstractClient::gameJoinedEventReceived, this, &TabSupervisor::localGameJoined);
}

/**
 * Call this when Cockatrice disconnects from the server in order to clean up.
 */
void TabSupervisor::stop()
{
    if ((!client) && localClients.isEmpty())
        return;

    resetTabsMenu();

    if (!localClients.isEmpty()) {
        for (int i = 0; i < localClients.size(); ++i)
            localClients[i]->deleteLater();
        localClients.clear();

        emit localGameEnded();
    } else {
        if (tabAccount) {
            tabAccount->closeRequest(true);
        }
        if (tabServer) {
            tabServer->closeRequest(true);
        }
        if (tabDeckStorage) {
            tabDeckStorage->closeRequest(true);
        }
        if (tabReplays) {
            tabReplays->closeRequest(true);
        }
        if (tabAdmin) {
            tabAdmin->closeRequest(true);
        }
        if (tabLog) {
            tabLog->closeRequest(true);
        }
    }

    QList<Tab *> tabsToDelete;

    for (auto i = roomTabs.cbegin(), end = roomTabs.cend(); i != end; ++i) {
        tabsToDelete << i.value();
    }

    for (auto i = gameTabs.cbegin(), end = gameTabs.cend(); i != end; ++i) {
        tabsToDelete << i.value();
    }

    for (auto i = messageTabs.cbegin(), end = messageTabs.cend(); i != end; ++i) {
        tabsToDelete << i.value();
    }

    for (const auto tab : tabsToDelete) {
        tab->closeRequest(true);
    }

    delete userInfo;
    userInfo = 0;
}

void TabSupervisor::actTabVisualDeckStorage(bool checked)
{
    if (checked && !tabVisualDeckStorage) {
        tabVisualDeckStorage = new TabDeckStorageVisual(this, client);
        myAddTab(tabVisualDeckStorage);
        setCurrentWidget(tabVisualDeckStorage);
        connect(tabVisualDeckStorage, &Tab::closed, this, [this] {
            tabVisualDeckStorage = nullptr;
            aTabVisualDeckStorage->setChecked(false);
        });
    } else if (!checked && tabVisualDeckStorage) {
        tabVisualDeckStorage->closeRequest();
    }
}

void TabSupervisor::actTabServer(bool checked)
{
    if (checked && !tabServer) {
        tabServer = new TabServer(this, client);
        connect(tabServer, &TabServer::roomJoined, this, &TabSupervisor::addRoomTab);
        myAddTab(tabServer);
        connect(tabServer, &Tab::closed, this, [this] {
            tabServer = nullptr;
            aTabServer->setChecked(false);
        });
    } else if (!checked && tabServer) {
        tabServer->closeRequest();
    }
}

void TabSupervisor::actTabAccount(bool checked)
{
    if (checked && !tabAccount) {
        tabAccount = new TabAccount(this, client, *userInfo);
        connect(tabAccount, &TabAccount::openMessageDialog, this, &TabSupervisor::addMessageTab);
        connect(tabAccount, &TabAccount::userJoined, this, &TabSupervisor::processUserJoined);
        connect(tabAccount, &TabAccount::userLeft, this, &TabSupervisor::processUserLeft);
        myAddTab(tabAccount);
        connect(tabAccount, &Tab::closed, this, [this] {
            tabAccount = nullptr;
            aTabAccount->setChecked(false);
        });
    } else if (!checked && tabAccount) {
        tabAccount->closeRequest();
    }
}

void TabSupervisor::actTabDeckStorage(bool checked)
{
    if (checked && !tabDeckStorage) {
        tabDeckStorage = new TabDeckStorage(this, client);
        connect(tabDeckStorage, &TabDeckStorage::openDeckEditor, this, &TabSupervisor::addDeckEditorTab);
        myAddTab(tabDeckStorage);
        connect(tabDeckStorage, &Tab::closed, this, [this] {
            tabDeckStorage = nullptr;
            aTabDeckStorage->setChecked(false);
        });
    } else if (!checked && tabDeckStorage) {
        tabDeckStorage->closeRequest();
    }
}

void TabSupervisor::actTabReplays(bool checked)
{
    if (checked && !tabReplays) {
        tabReplays = new TabReplays(this, client);
        connect(tabReplays, &TabReplays::openReplay, this, &TabSupervisor::openReplay);
        myAddTab(tabReplays);
        connect(tabReplays, &Tab::closed, this, [this] {
            tabReplays = nullptr;
            aTabReplays->setChecked(false);
        });
    } else if (!checked && tabReplays) {
        tabReplays->closeRequest();
    }
}

void TabSupervisor::actTabAdmin(bool checked)
{
    if (checked && !tabAdmin) {
        tabAdmin = new TabAdmin(this, client, (userInfo->user_level() & ServerInfo_User::IsAdmin));
        connect(tabAdmin, &TabAdmin::adminLockChanged, this, &TabSupervisor::adminLockChanged);
        myAddTab(tabAdmin);
        connect(tabAdmin, &Tab::closed, this, [this] {
            tabAdmin = nullptr;
            aTabAdmin->setChecked(false);
        });
    } else if (!checked && tabAdmin) {
        tabAdmin->closeRequest();
    }
}

void TabSupervisor::actTabLog(bool checked)
{
    if (checked && !tabLog) {
        tabLog = new TabLog(this, client);
        myAddTab(tabLog);
        connect(tabLog, &Tab::closed, this, [this] {
            tabLog = nullptr;
            aTabAdmin->setChecked(false);
        });
    } else if (!checked && tabLog) {
        tabLog->closeRequest();
    }
}

void TabSupervisor::updatePingTime(int value, int max)
{
    if (!tabServer)
        return;
    if (tabServer->getContentsChanged())
        return;

    setTabIcon(indexOf(tabServer), QIcon(PingPixmapGenerator::generatePixmap(15, value, max)));
}

void TabSupervisor::addCloseButtonToTab(Tab *tab, int tabIndex)
{
    auto closeSide = static_cast<QTabBar::ButtonPosition>(
        tabBar()->style()->styleHint(QStyle::SH_TabBar_CloseButtonPosition, nullptr, tabBar()));
    auto *closeButton = new CloseButton(tab);
    connect(closeButton, &CloseButton::clicked, tab, [tab] { tab->closeRequest(); });
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
            roomGameTypes.insert(event.game_types(i).game_type_id(),
                                 QString::fromStdString(event.game_types(i).description()));

    TabGame *tab = new TabGame(this, QList<AbstractClient *>() << client, event, roomGameTypes);
    connect(tab, &TabGame::gameClosing, this, &TabSupervisor::gameLeft);
    connect(tab, &TabGame::openMessageDialog, this, &TabSupervisor::addMessageTab);
    connect(tab, &TabGame::openDeckEditor, this, &TabSupervisor::addDeckEditorTab);
    myAddTab(tab);
    gameTabs.insert(event.game_info().game_id(), tab);
    setCurrentWidget(tab);
}

void TabSupervisor::localGameJoined(const Event_GameJoined &event)
{
    TabGame *tab = new TabGame(this, localClients, event, QMap<int, QString>());
    connect(tab, &TabGame::gameClosing, this, &TabSupervisor::gameLeft);
    connect(tab, &TabGame::openDeckEditor, this, &TabSupervisor::addDeckEditorTab);
    myAddTab(tab);
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
    connect(tab, &TabRoom::maximizeClient, this, &TabSupervisor::maximizeMainWindow);
    connect(tab, &TabRoom::roomClosing, this, &TabSupervisor::roomLeft);
    connect(tab, &TabRoom::openMessageDialog, this, &TabSupervisor::addMessageTab);
    myAddTab(tab);
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
    connect(replayTab, &TabGame::gameClosing, this, &TabSupervisor::replayLeft);
    myAddTab(replayTab);
    replayTabs.append(replayTab);
    setCurrentWidget(replayTab);
}

void TabSupervisor::replayLeft(TabGame *tab)
{
    if (tab == currentWidget())
        emit setMenu();

    replayTabs.removeOne(tab);
}

TabMessage *TabSupervisor::addMessageTab(const QString &receiverName, bool focus)
{
    if (receiverName == QString::fromStdString(userInfo->name()))
        return nullptr;

    ServerInfo_User otherUser;
    UserListTWI *twi = tabAccount->getAllUsersList()->getUsers().value(receiverName);
    if (twi)
        otherUser = twi->getUserInfo();
    else
        otherUser.set_name(receiverName.toStdString());

    TabMessage *tab;
    tab = messageTabs.value(QString::fromStdString(otherUser.name()));
    if (tab) {
        if (focus)
            setCurrentWidget(tab);
        return tab;
    }

    tab = new TabMessage(this, client, *userInfo, otherUser);
    connect(tab, &TabMessage::talkClosing, this, &TabSupervisor::talkLeft);
    connect(tab, &TabMessage::maximizeClient, this, &TabSupervisor::maximizeMainWindow);
    myAddTab(tab);
    messageTabs.insert(receiverName, tab);
    if (focus)
        setCurrentWidget(tab);
    return tab;
}

void TabSupervisor::maximizeMainWindow()
{
    emit showWindowIfHidden();
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
    connect(tab, &TabDeckEditor::deckEditorClosing, this, &TabSupervisor::deckEditorClosed);
    connect(tab, &TabDeckEditor::openDeckEditor, this, &TabSupervisor::addDeckEditorTab);
    myAddTab(tab);
    deckEditorTabs.append(tab);
    setCurrentWidget(tab);
    return tab;
}

void TabSupervisor::deckEditorClosed(TabDeckEditor *tab)
{
    if (tab == currentWidget())
        emit setMenu();

    deckEditorTabs.removeOne(tab);
    removeTab(indexOf(tab));
}

void TabSupervisor::tabUserEvent(bool globalEvent)
{
    Tab *tab = static_cast<Tab *>(sender());
    if (tab != currentWidget()) {
        tab->setContentsChanged(true);
        setTabIcon(indexOf(tab), QPixmap("theme:icons/tab_changed"));
    }
    if (globalEvent && SettingsCache::instance().getNotificationsEnabled())
        QApplication::alert(this);
}

void TabSupervisor::updateTabText(Tab *tab, const QString &newTabText)
{
    int idx = indexOf(tab);
    setTabText(idx, sanitizeTabName(newTabText));
    setTabToolTip(idx, sanitizeHtml(newTabText));
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
        tab->processGameEventContainer(cont, qobject_cast<AbstractClient *>(sender()), {});
    else
        qDebug() << "gameEvent: invalid gameId";
}

void TabSupervisor::processUserMessageEvent(const Event_UserMessage &event)
{
    QString senderName = QString::fromStdString(event.sender_name());
    TabMessage *tab = messageTabs.value(senderName);
    if (!tab)
        tab = messageTabs.value(QString::fromStdString(event.receiver_name()));
    if (!tab) {
        UserListTWI *twi = tabAccount->getAllUsersList()->getUsers().value(senderName);
        if (twi) {
            UserLevelFlags userLevel = UserLevelFlags(twi->getUserInfo().user_level());
            if (SettingsCache::instance().getIgnoreUnregisteredUserMessages() &&
                !userLevel.testFlag(ServerInfo_User::IsRegistered))
                // Flags are additive, so reg/mod/admin are all IsRegistered
                return;
        }
        tab = addMessageTab(QString::fromStdString(event.sender_name()), false);
    }
    if (!tab)
        return;
    tab->processUserMessageEvent(event);
}

void TabSupervisor::actShowPopup(const QString &message)
{
    qDebug() << "ACT SHOW POPUP";
    if (trayIcon && (QApplication::activeWindow() == nullptr || QApplication::focusWidget() == nullptr)) {
        qDebug() << "LAUNCHING POPUP";
        // disconnect(trayIcon, SIGNAL(messageClicked()), nullptr, nullptr);
        trayIcon->showMessage(message, tr("Click to view"));
        // connect(trayIcon, SIGNAL(messageClicked()), chatView, SLOT(actMessageClicked()));
    }
}

void TabSupervisor::processUserLeft(const QString &userName)
{
    TabMessage *tab = messageTabs.value(userName);
    if (tab)
        tab->processUserLeft();
}

void TabSupervisor::processUserJoined(const ServerInfo_User &userInfoJoined)
{
    QString userName = QString::fromStdString(userInfoJoined.name());
    if (isUserBuddy(userName)) {
        Tab *tab = static_cast<Tab *>(getTabAccount());

        if (tab != currentWidget()) {
            tab->setContentsChanged(true);
            QPixmap avatarPixmap =
                UserLevelPixmapGenerator::generatePixmap(13, (UserLevelFlags)userInfoJoined.user_level(), true,
                                                         QString::fromStdString(userInfoJoined.privlevel()));
            setTabIcon(indexOf(tab), QPixmap(avatarPixmap));
        }

        if (SettingsCache::instance().getBuddyConnectNotificationsEnabled()) {
            QApplication::alert(this);
            this->actShowPopup(tr("Your buddy %1 has signed on!").arg(userName));
        }
    }

    TabMessage *tab = messageTabs.value(userName);
    if (tab)
        tab->processUserJoined(userInfoJoined);
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
        tab->tabActivated();
    } else
        emit setMenu();
}

/**
 * Determine if a user is a moderator/administrator
 * By seeing if they have the admin tab open & unlocked
 * @return if the admin tab is open & unlocked
 */
bool TabSupervisor::getAdminLocked() const
{
    if (!tabAdmin)
        return true;
    return tabAdmin->getLocked();
}

void TabSupervisor::processNotifyUserEvent(const Event_NotifyUser &event)
{

    switch ((Event_NotifyUser::NotificationType)event.type()) {
        case Event_NotifyUser::UNKNOWN:
            QMessageBox::information(
                this, tr("Unknown Event"),
                tr("The server has sent you a message that your client does not understand.\nThis message might mean "
                   "there is a new version of Cockatrice available or this server is running a custom or pre-release "
                   "version.\n\nTo update your client, go to Help -> Check for Updates."));
            break;
        case Event_NotifyUser::IDLEWARNING:
            QMessageBox::information(this, tr("Idle Timeout"), tr("You are about to be logged out due to inactivity."));
            break;
        case Event_NotifyUser::PROMOTED:
            QMessageBox::information(
                this, tr("Promotion"),
                tr("You have been promoted. Please log out and back in for changes to take effect."));
            break;
        case Event_NotifyUser::WARNING: {
            if (!QString::fromStdString(event.warning_reason()).simplified().isEmpty())
                QMessageBox::warning(this, tr("Warned"),
                                     tr("You have received a warning due to %1.\nPlease refrain from engaging in this "
                                        "activity or further actions may be taken against you. If you have any "
                                        "questions, please private message a moderator.")
                                         .arg(QString::fromStdString(event.warning_reason()).simplified()));
            break;
        }
        case Event_NotifyUser::CUSTOM: {
            if (!QString::fromStdString(event.custom_title()).simplified().isEmpty() &&
                !QString::fromStdString(event.custom_content()).simplified().isEmpty()) {
                QMessageBox msgBox;
                msgBox.setParent(this);
                msgBox.setWindowFlags(Qt::Dialog);
                msgBox.setIcon(QMessageBox::Information);
                msgBox.setWindowTitle(QString::fromStdString(event.custom_title()).simplified());
                msgBox.setText(tr("You have received the following message from the server.\n(custom messages like "
                                  "these could be untranslated)"));
                msgBox.setDetailedText(QString::fromStdString(event.custom_content()).simplified());
                msgBox.setMinimumWidth(200);
                msgBox.exec();
            }
            break;
        }
        default:;
    }
}

bool TabSupervisor::isOwnUserRegistered() const
{
    return userInfo != nullptr && (userInfo->user_level() & ServerInfo_User::IsRegistered) != 0;
}

QString TabSupervisor::getOwnUsername() const
{
    return userInfo != nullptr ? QString::fromStdString(userInfo->name()) : QString();
}

bool TabSupervisor::isUserBuddy(const QString &userName) const
{
    if (!getTabAccount())
        return false;
    if (!getTabAccount()->getBuddyList())
        return false;
    QMap<QString, UserListTWI *> buddyList = getTabAccount()->getBuddyList()->getUsers();
    bool senderIsBuddy = buddyList.contains(userName);
    return senderIsBuddy;
}

bool TabSupervisor::isUserIgnored(const QString &userName) const
{
    if (!getTabAccount())
        return false;
    if (!getTabAccount()->getIgnoreList())
        return false;
    QMap<QString, UserListTWI *> buddyList = getTabAccount()->getIgnoreList()->getUsers();
    bool senderIsBuddy = buddyList.contains(userName);
    return senderIsBuddy;
}

const ServerInfo_User *TabSupervisor::getOnlineUser(const QString &userName) const
{
    if (!getTabAccount())
        return nullptr;
    if (!getTabAccount()->getAllUsersList())
        return nullptr;
    QMap<QString, UserListTWI *> userList = getTabAccount()->getAllUsersList()->getUsers();
    const QString &userNameToMatchLower = userName.toLower();
    QMap<QString, UserListTWI *>::iterator i;

    for (i = userList.begin(); i != userList.end(); ++i)
        if (i.key().toLower() == userNameToMatchLower) {
            const ServerInfo_User &_userInfo = i.value()->getUserInfo();
            return &_userInfo;
        }

    return nullptr;
};

bool TabSupervisor::switchToGameTabIfAlreadyExists(const int gameId)
{
    bool isGameTabExists = false;
    if (gameTabs.contains(gameId)) {
        isGameTabExists = true;
        TabGame *tabGame = gameTabs[gameId];
        const int gameTabIndex = indexOf(tabGame);
        setCurrentIndex(gameTabIndex);
    }

    return isGameTabExists;
}
