#include "tab_supervisor.h"

#include "../../../client/settings/cache_settings.h"
#include "../../../client/settings/shortcuts_settings.h"
#include "../../intents/intent_join_server_game.h"
#include "../interface/pixel_map_generator.h"
#include "../interface/widgets/server/game_link.h"
#include "../interface/widgets/server/user/user_list_manager.h"
#include "../interface/widgets/server/user/user_list_widget.h"
#include "../main.h"
#include "api/archidekt/tab_archidekt.h"
#include "api/edhrec/tab_edhrec_main.h"
#include "tab_account.h"
#include "tab_admin.h"
#include "tab_card_art_rules.h"
#include "tab_deck_editor.h"
#include "tab_deck_storage.h"
#include "tab_developer.h"
#include "tab_game.h"
#include "tab_home.h"
#include "tab_logs.h"
#include "tab_message.h"
#include "tab_moderation.h"
#include "tab_replays.h"
#include "tab_report.h"
#include "tab_room.h"
#include "tab_server.h"
#include "tab_visual_database_display.h"
#include "visual_deck_editor/tab_deck_editor_visual.h"
#include "visual_deck_editor/tab_deck_editor_visual_tab_widget.h"
#include "visual_deck_storage/tab_deck_storage_visual.h"

#include <QApplication>
#include <QMessageBox>
#include <QPainter>
#include <QSystemTrayIcon>
#include <libcockatrice/network/client/abstract/abstract_client.h>
#include <libcockatrice/network/client/remote/remote_client.h>
#include <libcockatrice/protocol/pb/event_game_joined.pb.h>
#include <libcockatrice/protocol/pb/event_notify_user.pb.h>
#include <libcockatrice/protocol/pb/event_user_message.pb.h>
#include <libcockatrice/protocol/pb/game_event_container.pb.h>
#include <libcockatrice/protocol/pb/room_commands.pb.h>
#include <libcockatrice/protocol/pb/room_event.pb.h>
#include <libcockatrice/protocol/pb/serverinfo_room.pb.h>
#include <libcockatrice/protocol/pb/serverinfo_user.pb.h>
#include <libcockatrice/protocol/pending_command.h>
#include <libcockatrice/settings/chat_settings.h>
#include <libcockatrice/settings/deck_editor_settings.h>
#include <libcockatrice/settings/interface_settings.h>
#include <libcockatrice/settings/tabs_settings.h>

QRect MacOSTabFixStyle::subElementRect(SubElement element, const QStyleOption *option, const QWidget *widget) const
{
    if (element != SE_TabBarTabText) {
        return QProxyStyle::subElementRect(element, option, widget);
    }

    // Skip over QProxyStyle handling subElementRect,
    // This fixes an issue on OSX where the labels for tabs with a button and an icon
    // get cut-off too early
    return QCommonStyle::subElementRect(element, option, widget);
}

CloseButton::CloseButton(QWidget *parent) : QAbstractButton(parent)
{
    setFocusPolicy(Qt::NoFocus);
    setCursor(Qt::ArrowCursor);
    resize(this->sizeHint());
}

QSize CloseButton::sizeHint() const
{
    ensurePolished();
    int width = style()->pixelMetric(QStyle::PM_TabCloseIndicatorWidth, nullptr, this);
    int height = style()->pixelMetric(QStyle::PM_TabCloseIndicatorHeight, nullptr, this);
    return {width, height};
}

void CloseButton::enterEvent(QEnterEvent *event)
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
    if (isEnabled() && underMouse() && !isChecked() && !isDown()) {
        opt.state |= QStyle::State_Raised;
    }
    if (isChecked()) {
        opt.state |= QStyle::State_On;
    }
    if (isDown()) {
        opt.state |= QStyle::State_Sunken;
    }

    if (const auto *tb = qobject_cast<const QTabBar *>(parent())) {
        int index = tb->currentIndex();
        auto position = (QTabBar::ButtonPosition)style()->styleHint(QStyle::SH_TabBar_CloseButtonPosition, nullptr, tb);
        if (tb->tabButton(index, position) == this) {
            opt.state |= QStyle::State_Selected;
        }
    }

    style()->drawPrimitive(QStyle::PE_IndicatorTabClose, &opt, &p, this);
}

TabSupervisor::TabSupervisor(AbstractClient *_client, QMenu *tabsMenu, QWidget *parent)
    : QTabWidget(parent), userInfo(nullptr), client(_client), tabsMenu(tabsMenu), tabVisualDeckStorage(nullptr),
      tabServer(nullptr), tabAccount(nullptr), tabDeckStorage(nullptr), tabReplays(nullptr), tabAdmin(nullptr),
      tabLog(nullptr), tabReport(nullptr), tabModeration(nullptr), tabDeveloper(nullptr), isLocalGame(false)
{
    setElideMode(Qt::ElideRight);
    setMovable(true);
    setIconSize(QSize(15, 15));

#if defined(Q_OS_MAC)
    // This is necessary to fix an issue on macOS,
    // where tabs with icons and buttons get drawn incorrectly
    tabBar()->setStyle(new MacOSTabFixStyle);
#endif

    userListManager = new UserListManager(client, this);

    // connect tab changes
    connect(this, &TabSupervisor::currentChanged, this, &TabSupervisor::updateCurrent);

    // connect client
    connect(client, &AbstractClient::roomEventReceived, this, &TabSupervisor::processRoomEvent);
    connect(client, &AbstractClient::gameEventContainerReceived, this, &TabSupervisor::processGameEventContainer);
    connect(client, &AbstractClient::gameJoinedEventReceived, this, &TabSupervisor::gameJoined);
    connect(client, &AbstractClient::userMessageEventReceived, this, &TabSupervisor::processUserMessageEvent);
    connect(client, &AbstractClient::maxPingTime, this, &TabSupervisor::updatePingTime);
    connect(client, &AbstractClient::pingStatsUpdated, this, &TabSupervisor::updateLatencyTooltip);
    connect(client, &AbstractClient::notifyUserEventReceived, this, &TabSupervisor::processNotifyUserEvent);

    // create tabs menu actions
    aTabDeckEditor = new QAction(this);
    connect(aTabDeckEditor, &QAction::triggered, this, [this] { addDeckEditorTab(LoadedDeck()); });

    aTabVisualDeckEditor = new QAction(this);
    connect(aTabVisualDeckEditor, &QAction::triggered, this, [this] { addVisualDeckEditorTab(LoadedDeck()); });

    aTabEdhRec = new QAction(this);
    connect(aTabEdhRec, &QAction::triggered, this, [this] { addEdhrecMainTab(); });

    aTabArchidekt = new QAction(this);
    connect(aTabArchidekt, &QAction::triggered, this, [this] { addArchidektTab(); });

    aTabHome = new QAction(this);
    aTabHome->setCheckable(true);
    connect(aTabHome, &QAction::triggered, this, &TabSupervisor::actTabHome);

    aTabVisualDeckStorage = new QAction(this);
    aTabVisualDeckStorage->setCheckable(true);
    connect(aTabVisualDeckStorage, &QAction::triggered, this, &TabSupervisor::actTabVisualDeckStorage);

    aTabVisualDatabaseDisplay = new QAction(this);
    connect(aTabVisualDatabaseDisplay, &QAction::triggered, this, [this] { addVisualDatabaseDisplayTab(); });

    aTabServer = new QAction(this);
    aTabServer->setCheckable(true);
    connect(aTabServer, &QAction::triggered, this, &TabSupervisor::actTabServer);

    aTabAccount = new QAction(this);
    aTabAccount->setCheckable(true);
    connect(aTabAccount, &QAction::triggered, this, &TabSupervisor::actTabAccount);

    aTabDeckStorage = new QAction(this);
    aTabDeckStorage->setCheckable(true);
    connect(aTabDeckStorage, &QAction::triggered, this, &TabSupervisor::actTabDeckStorage);

    aTabReplays = new QAction(this);
    aTabReplays->setCheckable(true);
    connect(aTabReplays, &QAction::triggered, this, &TabSupervisor::actTabReplays);

    aTabAdmin = new QAction(this);
    aTabAdmin->setCheckable(true);
    connect(aTabAdmin, &QAction::triggered, this, &TabSupervisor::actTabAdmin);

    aTabCardArtRules = new QAction(this);
    aTabCardArtRules->setCheckable(true);
    connect(aTabCardArtRules, &QAction::triggered, this, &TabSupervisor::actTabCardArtRules);

    aTabLog = new QAction(this);
    aTabLog->setCheckable(true);
    connect(aTabLog, &QAction::triggered, this, &TabSupervisor::actTabLog);

    aTabReport = new QAction(this);
    aTabReport->setCheckable(true);
    connect(aTabReport, &QAction::triggered, this, &TabSupervisor::actTabReport);

    aTabModeration = new QAction(this);
    aTabModeration->setCheckable(true);
    connect(aTabModeration, &QAction::triggered, this, &TabSupervisor::actTabModeration);

    aTabDeveloper = new QAction(this);
    aTabDeveloper->setCheckable(true);
    connect(aTabDeveloper, &QAction::triggered, this, &TabSupervisor::actTabDeveloper);

    connect(&SettingsCache::instance().shortcuts(), &ShortcutsSettings::shortCutChanged, this,
            &TabSupervisor::refreshShortcuts);
    refreshShortcuts();

    resetTabsMenu();

    retranslateUi();
}

TabSupervisor::~TabSupervisor()
{
    // Note: this used to call stop(), but stop() is doing a bunch of stuff including emitting some signals,
    // and we don't want to do that in a destructor.

    for (auto &localClient : localClients) {
        localClient->deleteLater();
    }
    localClients.clear();

    delete userInfo;
    userInfo = nullptr;
}

void TabSupervisor::retranslateUi()
{
    // tab menu actions
    aTabDeckEditor->setText(tr("Deck Editor"));
    aTabVisualDeckEditor->setText(tr("Visual Deck Editor"));
    aTabEdhRec->setText(tr("EDHRec"));
    aTabArchidekt->setText(tr("Archidekt"));
    aTabHome->setText(tr("Home"));
    aTabVisualDeckStorage->setText(tr("&Visual Deck Storage"));
    aTabVisualDatabaseDisplay->setText(tr("Visual Database Display"));
    aTabServer->setText(tr("Server"));
    aTabAccount->setText(tr("Account"));
    aTabDeckStorage->setText(tr("Deck Storage"));
    aTabReplays->setText(tr("Game Replays"));
    aTabAdmin->setText(tr("Administration"));
    aTabLog->setText(tr("Logs"));
    aTabReport->setText(tr("Report Queue"));
    aTabModeration->setText(tr("Moderation"));
    aTabDeveloper->setText(tr("Developer"));

    // tabs
    QList<Tab *> tabs;
    tabs.append(tabServer);
    tabs.append(tabReplays);
    tabs.append(tabDeckStorage);
    tabs.append(tabAdmin);
    tabs.append(tabAccount);
    tabs.append(tabLog);
    tabs.append(tabReport);
    tabs.append(tabModeration);
    tabs.append(tabDeveloper);
    QMapIterator<int, TabRoom *> roomIterator(roomTabs);
    while (roomIterator.hasNext()) {
        tabs.append(roomIterator.next().value());
    }
    QMapIterator<int, TabGame *> gameIterator(gameTabs);
    while (gameIterator.hasNext()) {
        tabs.append(gameIterator.next().value());
    }
    QListIterator<TabGame *> replayIterator(replayTabs);
    while (replayIterator.hasNext()) {
        tabs.append(replayIterator.next());
    }
    QListIterator<AbstractTabDeckEditor *> deckEditorIterator(deckEditorTabs);
    while (deckEditorIterator.hasNext()) {
        tabs.append(deckEditorIterator.next());
    }
    QMapIterator<QString, TabMessage *> messageIterator(messageTabs);
    while (messageIterator.hasNext()) {
        tabs.append(messageIterator.next().value());
    }

    for (auto &tab : tabs) {
        if (tab) {
            int idx = indexOf(tab);
            QString tabText = tab->getTabText();
            setTabText(idx, sanitizeTabName(tabText));
            setTabToolTip(idx, sanitizeHtml(tabText));
            tab->retranslateUi();
        }
    }
}

void TabSupervisor::refreshShortcuts()
{
    ShortcutsSettings &shortcuts = SettingsCache::instance().shortcuts();
    aTabDeckEditor->setShortcuts(shortcuts.getShortcut("Tabs/aTabDeckEditor"));
    aTabVisualDeckEditor->setShortcuts(shortcuts.getShortcut("Tabs/aTabVisualDeckEditor"));

    aTabHome->setShortcuts(shortcuts.getShortcut("Tabs/aTabHome"));
    aTabVisualDeckStorage->setShortcuts(shortcuts.getShortcut("Tabs/aTabVisualDeckStorage"));
    aTabServer->setShortcuts(shortcuts.getShortcut("Tabs/aTabServer"));
    aTabAccount->setShortcuts(shortcuts.getShortcut("Tabs/aTabAccount"));
    aTabDeckStorage->setShortcuts(shortcuts.getShortcut("Tabs/aTabDeckStorage"));
    aTabReplays->setShortcuts(shortcuts.getShortcut("Tabs/aTabReplays"));
    aTabAdmin->setShortcuts(shortcuts.getShortcut("Tabs/aTabAdmin"));
    aTabLog->setShortcuts(shortcuts.getShortcut("Tabs/aTabLog"));
    aTabReport->setShortcuts(shortcuts.getShortcut("Tabs/aTabReport"));
    aTabModeration->setShortcuts(shortcuts.getShortcut("Tabs/aTabModeration"));
}

void TabSupervisor::closeEvent(QCloseEvent *event)
{
    // This will accept the event, which we may then override.
    QTabWidget::closeEvent(event);

    if (getGameCount()) {
        if (QMessageBox::question(this, tr("Are you sure?"),
                                  tr("There are still open games. Are you sure you want to quit?"),
                                  QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::No) {
            event->ignore();
            return;
        }
    }

    for (AbstractTabDeckEditor *tab : deckEditorTabs) {
        if (!tab->confirmClose()) {
            event->ignore();
        }
    }
}

AbstractClient *TabSupervisor::getClient() const
{
    return localClients.isEmpty() ? client : localClients.first();
}

QString TabSupervisor::sanitizeTabName(QString dirty)
{
    return dirty.replace("&", "&&");
}

QString TabSupervisor::sanitizeHtml(QString dirty)
{
    return dirty.replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;").replace("\"", "&quot;");
}

/**
 * If the action is not in the target checked state, then set it to that state by triggering the action.
 * If the action is already in the target checked state, then do nothing.
 *
 * This allows us to programmatically trigger a QAction::triggered signal for a specific checked state.
 */
static void checkAndTrigger(QAction *checkableAction, bool checked)
{
    if (checkableAction->isChecked() != checked) {
        checkableAction->trigger();
    }
}

/**
 * Opens the always-available tabs, depending on settings, and lands on the configured startup tab.
 *
 * The startup destination is a request: tabs that were not open before (deck editors, storage
 * tabs disabled in the Tabs menu) are opened as part of the startup flow. Destinations that
 * require a server connection (Server, Server Room) are handled asynchronously by MainWindow
 * through the intent system, since this class has no RemoteClient.
 */
void TabSupervisor::initStartupTabs()
{
    openTabHome();
    setCurrentWidget(tabHome);

    if (SettingsCache::instance().tabs().getTabVisualDeckStorageOpen()) {
        openTabVisualDeckStorage();
    }
    if (SettingsCache::instance().tabs().getTabDeckStorageOpen()) {
        openTabDeckStorage();
    }
    if (SettingsCache::instance().tabs().getTabReplaysOpen()) {
        openTabReplays();
    }

    switch (SettingsCache::instance().tabs().getStartupTabIndex()) {
        case StartupTab::StartupTabVisualDeckStorage:
            if (!tabVisualDeckStorage) {
                openTabVisualDeckStorage();
            }
            setCurrentWidget(tabVisualDeckStorage);
            break;
        case StartupTab::StartupTabDeckStorage:
            if (!tabDeckStorage) {
                openTabDeckStorage();
            }
            setCurrentWidget(tabDeckStorage);
            break;
        case StartupTab::StartupTabReplays:
            if (!tabReplays) {
                openTabReplays();
            }
            setCurrentWidget(tabReplays);
            break;
        case StartupTab::StartupTabDeckEditor:
            addDeckEditorTab(LoadedDeck());
            break;
        case StartupTab::StartupTabVisualDeckEditor:
            addVisualDeckEditorTab(LoadedDeck());
            break;
        case StartupTab::StartupTabServer:
        case StartupTab::StartupTabServerRoom:
            // Handled asynchronously by MainWindow::applyStartupDestination(); Home stays selected
            // until the server connection succeeds.
            break;
        case StartupTab::StartupTabHome:
        default:
            setCurrentWidget(tabHome);
            break;
    }
}

/**
 * Adds the tab to the TabSupervisor's tab bar.
 *
 * @param tab The Tab to add
 * @param manager The menu action that corresponds to this tab, if this is a single-instance managed tab. Pass in
 * nullptr if this is not a managed tab.
 * @return The index of the added tab in the tab widget's tab menu
 */
int TabSupervisor::myAddTab(Tab *tab, QAction *manager)
{
    connect(tab, &TabGame::userEvent, this, &TabSupervisor::tabUserEvent);
    connect(tab, &TabGame::tabTextChanged, this, &TabSupervisor::updateTabText);
    connect(tab, &TabGame::cockatriceLinkActivated, this, &TabSupervisor::cockatriceLinkActivated);

    QString tabText = tab->getTabText();
    int idx = addTab(tab, sanitizeTabName(tabText));
    setTabToolTip(idx, sanitizeHtml(tabText));

    addCloseButtonToTab(tab, idx, manager);

    return idx;
}

/**
 * Adds a usable close button to the tab.
 *
 * @param tab The Tab
 * @param tabIndex The tab bar index of the tab
 * @param manager The menu action that corresponds to this tab, if this is a single-instance managed tab. Pass in
 * nullptr if this is not a managed tab.
 */
void TabSupervisor::addCloseButtonToTab(Tab *tab, int tabIndex, QAction *manager)
{
    auto closeSide = static_cast<QTabBar::ButtonPosition>(
        tabBar()->style()->styleHint(QStyle::SH_TabBar_CloseButtonPosition, nullptr, tabBar()));
    auto *closeButton = new CloseButton(tab);
    if (manager) {
        // If managed, all close requests should go through the menu action
        connect(closeButton, &CloseButton::clicked, this, [manager] { checkAndTrigger(manager, false); });
    } else {
        connect(closeButton, &CloseButton::clicked, tab, &Tab::closeRequest);
    }
    tabBar()->setTabButton(tabIndex, closeSide, closeButton);
}

/**
 * Resets the tabs menu to the tabs that are always available
 */
void TabSupervisor::resetTabsMenu()
{
    tabsMenu->clear();
    tabsMenu->addAction(aTabDeckEditor);
    tabsMenu->addAction(aTabVisualDeckEditor);
    tabsMenu->addAction(aTabEdhRec);
    tabsMenu->addAction(aTabArchidekt);
    tabsMenu->addSeparator();
    tabsMenu->addAction(aTabHome);
    tabsMenu->addAction(aTabVisualDeckStorage);
    tabsMenu->addAction(aTabVisualDatabaseDisplay);
    tabsMenu->addAction(aTabDeckStorage);
    tabsMenu->addAction(aTabReplays);
}

void TabSupervisor::start(const ServerInfo_User &_userInfo)
{
    isLocalGame = false;
    userInfo = new ServerInfo_User(_userInfo);

    userListManager->handleConnect();

    resetTabsMenu();

    tabsMenu->addSeparator();
    tabsMenu->addAction(aTabServer);
    tabsMenu->addAction(aTabAccount);

    if (SettingsCache::instance().tabs().getTabServerOpen()) {
        openTabServer();
    }
    if (SettingsCache::instance().tabs().getTabAccountOpen()) {
        openTabAccount();
    }

    updatePingTime(0, -1);

    if (userInfo->user_level() & ServerInfo_User::IsModerator) {
        tabsMenu->addSeparator();
        tabsMenu->addAction(aTabAdmin);
        tabsMenu->addAction(aTabLog);
        tabsMenu->addAction(aTabCardArtRules);
        tabsMenu->addAction(aTabReport);
        tabsMenu->addAction(aTabModeration);

        if (SettingsCache::instance().tabs().getTabAdminOpen()) {
            openTabAdmin();
        }
        if (SettingsCache::instance().tabs().getTabLogOpen()) {
            openTabLog();
        }
        if (SettingsCache::instance().tabs().getTabReportOpen()) {
            openTabReport();
        }
        if (SettingsCache::instance().tabs().getTabModerationOpen()) {
            openTabModeration();
        }
        openTabCardArtRules();
    }

    if (userInfo->user_level() & ServerInfo_User::IsDeveloper) {
        tabsMenu->addSeparator();
        tabsMenu->addAction(aTabDeveloper);
        // Developers without moderation rights get log access through their
        // own role. Moderators already have the Logs entry from above.
        if (!(userInfo->user_level() & ServerInfo_User::IsModerator)) {
            tabsMenu->addAction(aTabLog);
            if (SettingsCache::instance().tabs().getTabLogOpen()) {
                openTabLog();
            }
        }
    }

    retranslateUi();
}

void TabSupervisor::startLocal(const QList<AbstractClient *> &_clients)
{
    resetTabsMenu();

    tabAccount = nullptr;
    tabAdmin = nullptr;
    tabLog = nullptr;
    tabReport = nullptr;
    tabModeration = nullptr;
    tabDeveloper = nullptr;
    isLocalGame = true;
    userInfo = new ServerInfo_User;
    localClients = _clients;
    for (int i = 0; i < localClients.size(); ++i) {
        connect(localClients[i], &AbstractClient::gameEventContainerReceived, this,
                &TabSupervisor::processGameEventContainer);
    }
    connect(localClients.first(), &AbstractClient::gameJoinedEventReceived, this, &TabSupervisor::localGameJoined);
}

/**
 * Call this when Cockatrice disconnects from the server in order to clean up.
 */
void TabSupervisor::stop()
{
    if ((!client) && localClients.isEmpty()) {
        return;
    }

    resetTabsMenu();

    if (!localClients.isEmpty()) {
        for (auto &localClient : localClients) {
            localClient->deleteLater();
        }
        localClients.clear();

        emit localGameEnded();
    } else {
        if (tabAccount) {
            tabAccount->close();
        }
        if (tabServer) {
            tabServer->close();
        }
        if (tabAdmin) {
            tabAdmin->close();
        }
        if (tabLog) {
            tabLog->close();
        }
        if (tabReport) {
            tabReport->close();
        }
        if (tabModeration) {
            tabModeration->close();
        }
        if (tabDeveloper) {
            tabDeveloper->close();
        }
    }

    QList<Tab *> tabsToDelete;

    for (auto i = roomTabs.cbegin(), end = roomTabs.cend(); i != end; ++i) {
        tabsToDelete << i.value();
    }

    for (auto i = gameTabs.cbegin(), end = gameTabs.cend(); i != end; ++i) {
        tabsToDelete << i.value();
    }

    for (const auto tab : tabsToDelete) {
        tab->close();
    }

    userListManager->handleDisconnect();

    delete userInfo;
    userInfo = nullptr;
}

void TabSupervisor::actTabHome(bool checked)
{
    if (checked && !tabHome) {
        openTabHome();
        setCurrentWidget(tabHome);
    } else if (!checked && tabHome) {
        tabHome->closeRequest();
    }
}

void TabSupervisor::openTabHome()
{
    tabHome = new TabHome(this, client);
    myAddTab(tabHome, aTabHome);
    connect(tabHome, &QObject::destroyed, this, [this] {
        tabHome = nullptr;
        aTabHome->setChecked(false);
    });
    aTabHome->setChecked(true);
}

void TabSupervisor::actTabVisualDeckStorage(bool checked)
{
    SettingsCache::instance().tabs().setTabVisualDeckStorageOpen(checked);
    if (checked && !tabVisualDeckStorage) {
        openTabVisualDeckStorage();
        setCurrentWidget(tabVisualDeckStorage);
    } else if (checked && tabVisualDeckStorage) {
        setCurrentWidget(tabVisualDeckStorage);
    } else if (!checked && tabVisualDeckStorage) {
        tabVisualDeckStorage->closeRequest();
    }
}

void TabSupervisor::openTabVisualDeckStorage()
{
    tabVisualDeckStorage = new TabDeckStorageVisual(this);
    myAddTab(tabVisualDeckStorage, aTabVisualDeckStorage);
    connect(tabVisualDeckStorage, &QObject::destroyed, this, [this] {
        tabVisualDeckStorage = nullptr;
        aTabVisualDeckStorage->setChecked(false);
    });
    aTabVisualDeckStorage->setChecked(true);
}

void TabSupervisor::actTabServer(bool checked)
{
    SettingsCache::instance().tabs().setTabServerOpen(checked);
    if (checked && !tabServer) {
        openTabServer();
        setCurrentWidget(tabServer);
    } else if (!checked && tabServer) {
        tabServer->closeRequest();
    }
}

void TabSupervisor::openTabServer()
{
    SettingsCache::instance().tabs().setTabServerOpen(true);
    if (tabServer) {
        return;
    }

    tabServer = new TabServer(this, client);
    connect(tabServer, &TabServer::roomJoined, this, &TabSupervisor::addRoomTab);
    myAddTab(tabServer, aTabServer);
    connect(tabServer, &QObject::destroyed, this, [this] {
        tabServer = nullptr;
        aTabServer->setChecked(false);
    });
    aTabServer->setChecked(true);
}

void TabSupervisor::actTabAccount(bool checked)
{
    SettingsCache::instance().tabs().setTabAccountOpen(checked);
    if (checked && !tabAccount) {
        openTabAccount();
        setCurrentWidget(tabAccount);
    } else if (!checked && tabAccount) {
        tabAccount->closeRequest();
    }
}

void TabSupervisor::openTabAccount()
{
    tabAccount = new TabAccount(this, client, *userInfo);
    connect(tabAccount, &TabAccount::openMessageDialog, this, &TabSupervisor::addMessageTab);
    connect(tabAccount, &TabAccount::userJoined, this, &TabSupervisor::processUserJoined);
    connect(tabAccount, &TabAccount::userLeft, this, &TabSupervisor::processUserLeft);
    myAddTab(tabAccount, aTabAccount);
    connect(tabAccount, &QObject::destroyed, this, [this] {
        tabAccount = nullptr;
        aTabAccount->setChecked(false);
    });
    aTabAccount->setChecked(true);
}

void TabSupervisor::actTabDeckStorage(bool checked)
{
    SettingsCache::instance().tabs().setTabDeckStorageOpen(checked);
    if (checked && !tabDeckStorage) {
        openTabDeckStorage();
        setCurrentWidget(tabDeckStorage);
    } else if (!checked && tabDeckStorage) {
        tabDeckStorage->closeRequest();
    }
}

void TabSupervisor::openTabDeckStorage()
{
    tabDeckStorage = new TabDeckStorage(this, client, userInfo);
    connect(tabDeckStorage, &TabDeckStorage::openDeckEditor, this, &TabSupervisor::openDeckInNewTab);
    myAddTab(tabDeckStorage, aTabDeckStorage);
    connect(tabDeckStorage, &QObject::destroyed, this, [this] {
        tabDeckStorage = nullptr;
        aTabDeckStorage->setChecked(false);
    });
    aTabDeckStorage->setChecked(true);
}

void TabSupervisor::actTabReplays(bool checked)
{
    SettingsCache::instance().tabs().setTabReplaysOpen(checked);
    if (checked && !tabReplays) {
        openTabReplays();
        setCurrentWidget(tabReplays);
    } else if (!checked && tabReplays) {
        tabReplays->closeRequest();
    } else if (checked && tabReplays) {
        setCurrentWidget(tabReplays);
    }
}

void TabSupervisor::openTabReplays()
{
    tabReplays = new TabReplays(this, client, userInfo);
    connect(tabReplays, &TabReplays::openReplay, this, &TabSupervisor::openReplay);
    myAddTab(tabReplays, aTabReplays);
    connect(tabReplays, &QObject::destroyed, this, [this] {
        tabReplays = nullptr;
        aTabReplays->setChecked(false);
    });
    aTabReplays->setChecked(true);
}

void TabSupervisor::actTabAdmin(bool checked)
{
    SettingsCache::instance().tabs().setTabAdminOpen(checked);
    if (checked && !tabAdmin) {
        openTabAdmin();
        setCurrentWidget(tabAdmin);
    } else if (!checked && tabAdmin) {
        tabAdmin->closeRequest();
    }
}

void TabSupervisor::openTabAdmin()
{
    tabAdmin = new TabAdmin(this, client, (userInfo->user_level() & ServerInfo_User::IsAdmin));
    connect(tabAdmin, &TabAdmin::adminLockChanged, this, &TabSupervisor::adminLockChanged);
    myAddTab(tabAdmin, aTabAdmin);
    connect(tabAdmin, &QObject::destroyed, this, [this] {
        tabAdmin = nullptr;
        aTabAdmin->setChecked(false);
    });
    aTabAdmin->setChecked(true);
}

void TabSupervisor::actTabCardArtRules(bool checked)
{
    if (checked && !tabCardArtRules) {
        openTabCardArtRules();
        setCurrentWidget(tabCardArtRules);
    } else if (!checked && tabCardArtRules) {
        tabCardArtRules->closeRequest();
    }
}

void TabSupervisor::openTabCardArtRules()
{
    tabCardArtRules = new TabCardArtRules(this, client);

    myAddTab(tabCardArtRules, aTabCardArtRules);

    connect(tabCardArtRules, &QObject::destroyed, this, [this] {
        tabCardArtRules = nullptr;
        aTabCardArtRules->setChecked(false);
    });

    aTabCardArtRules->setChecked(true);
}

void TabSupervisor::actTabLog(bool checked)
{
    SettingsCache::instance().tabs().setTabLogOpen(checked);
    if (checked && !tabLog) {
        openTabLog();
        setCurrentWidget(tabLog);
    } else if (!checked && tabLog) {
        tabLog->closeRequest();
    }
}

void TabSupervisor::openTabLog()
{
    // Developers query logs through the developer command family, so tell the
    // tab which family to use. The moderator family is strictly stronger, so a
    // moderator who also holds the developer bit keeps the moderator path — the
    // developer bit only selects the (narrowed) developer family on its own.
    const bool useDeveloperCommands = (userInfo->user_level() & ServerInfo_User::IsDeveloper) &&
                                      !(userInfo->user_level() & ServerInfo_User::IsModerator);
    tabLog = new TabLog(this, client, useDeveloperCommands);
    myAddTab(tabLog, aTabLog);
    connect(tabLog, &QObject::destroyed, this, [this] {
        tabLog = nullptr;
        aTabLog->setChecked(false);
    });
    aTabLog->setChecked(true);
}

void TabSupervisor::actTabReport(bool checked)
{
    SettingsCache::instance().tabs().setTabReportOpen(checked);
    if (checked && !tabReport) {
        openTabReport();
        setCurrentWidget(tabReport);
    } else if (!checked && tabReport) {
        tabReport->closeRequest();
    }
}

void TabSupervisor::openTabReport()
{
    tabReport = new TabReport(this, client);
    myAddTab(tabReport, aTabReport);
    connect(tabReport, &TabReport::openReplay, this, &TabSupervisor::openReplay);
    connect(tabReport, &TabReport::requestJoinGame, this, &TabSupervisor::joinReportGame);
    connect(tabReport, &QObject::destroyed, this, [this] {
        tabReport = nullptr;
        aTabReport->setChecked(false);
    });
    aTabReport->setChecked(true);
}

void TabSupervisor::actTabModeration(bool checked)
{
    SettingsCache::instance().tabs().setTabModerationOpen(checked);
    if (checked && !tabModeration) {
        openTabModeration();
        setCurrentWidget(tabModeration);
    } else if (!checked && tabModeration) {
        tabModeration->closeRequest();
    }
}

void TabSupervisor::openTabModeration(const QString &userName)
{
    if (tabModeration) {
        setCurrentWidget(tabModeration);
        if (!userName.isEmpty()) {
            tabModeration->investigate(userName);
        }
        return;
    }
    tabModeration = new TabModeration(this, client, userName);
    myAddTab(tabModeration, aTabModeration);
    connect(tabModeration, &QObject::destroyed, this, [this] {
        tabModeration = nullptr;
        aTabModeration->setChecked(false);
    });
    aTabModeration->setChecked(true);
}

void TabSupervisor::actTabDeveloper(bool checked)
{
    if (checked && !tabDeveloper) {
        openTabDeveloper();
        setCurrentWidget(tabDeveloper);
    } else if (!checked && tabDeveloper) {
        tabDeveloper->closeRequest();
    }
}

void TabSupervisor::openTabDeveloper()
{
    tabDeveloper = new TabDeveloper(this, client);
    myAddTab(tabDeveloper, aTabDeveloper);
    connect(tabDeveloper, &QObject::destroyed, this, [this] {
        tabDeveloper = nullptr;
        aTabDeveloper->setChecked(false);
    });
    aTabDeveloper->setChecked(true);
}

void TabSupervisor::updatePingTime(int value, int max)
{
    if (!tabServer) {
        return;
    }
    if (tabServer->getContentsChanged()) {
        return;
    }

    setTabIcon(indexOf(tabServer), QIcon(PingPixmapGenerator::generatePixmap(15, value, max)));
}

void TabSupervisor::updateLatencyTooltip(const LatencyTracker::Stats &stats)
{
    if (!tabServer) {
        return;
    }

    if (stats.sampleCount == 0) {
        setTabToolTip(indexOf(tabServer), QString());
        return;
    }

    setTabToolTip(indexOf(tabServer),
                  tr("Connection quality over the last %n sample(s):", "", stats.sampleCount) + "\n" +
                      tr("Last: %1 ms").arg(stats.lastMs) + "\n" + tr("Median: %1 ms").arg(stats.medianMs) + "\n" +
                      tr("95th percentile: %1 ms").arg(stats.p95Ms) + "\n" + tr("Maximum: %1 ms").arg(stats.maxMs));
}

void TabSupervisor::gameJoined(const Event_GameJoined &event)
{
    QMap<int, QString> roomGameTypes;
    TabRoom *room = roomTabs.value(event.game_info().room_id());
    if (room) {
        roomGameTypes = room->getGameTypes();
    } else {
        for (int i = 0; i < event.game_types_size(); ++i) {
            roomGameTypes.insert(event.game_types(i).game_type_id(),
                                 QString::fromStdString(event.game_types(i).description()));
        }
    }

    auto *tab = new TabGame(this, QList<AbstractClient *>() << client, event, roomGameTypes);
    connect(tab, &TabGame::gameClosing, this, &TabSupervisor::gameLeft);
    connect(tab, &TabGame::openMessageDialog, this, &TabSupervisor::addMessageTab);
    connect(tab, &TabGame::openDeckEditor, this, &TabSupervisor::openDeckInNewTab);
    myAddTab(tab);
    gameTabs.insert(event.game_info().game_id(), tab);
    setCurrentWidget(tab);
}

void TabSupervisor::localGameJoined(const Event_GameJoined &event)
{
    auto *tab = new TabGame(this, localClients, event, QMap<int, QString>());
    connect(tab, &TabGame::gameClosing, this, &TabSupervisor::gameLeft);
    connect(tab, &TabGame::openDeckEditor, this, &TabSupervisor::openDeckInNewTab);
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
    if (tab == currentWidget()) {
        emit setMenu();
    }

    gameTabs.remove(tab->getGame()->getGameMetaInfo()->gameId());
    removeTab(indexOf(tab));

    if (!localClients.isEmpty()) {
        stop();
    }
}

void TabSupervisor::addRoomTab(const ServerInfo_Room &info, bool setCurrent)
{
    auto *tab = new TabRoom(this, client, userInfo, info);
    connect(tab, &TabRoom::maximizeClient, this, &TabSupervisor::maximizeMainWindow);
    connect(tab, &TabRoom::roomClosing, this, &TabSupervisor::roomLeft);
    connect(tab, &TabRoom::openMessageDialog, this, &TabSupervisor::addMessageTab);
    connect(tab, &TabRoom::cockatriceLinkActivated, this, &TabSupervisor::cockatriceLinkActivated);
    myAddTab(tab);
    roomTabs.insert(info.room_id(), tab);
    if (setCurrent) {
        setCurrentWidget(tab);
    }
}

void TabSupervisor::roomLeft(TabRoom *tab)
{
    if (tab == currentWidget()) {
        emit setMenu();
    }

    roomTabs.remove(tab->getRoomId());
    removeTab(indexOf(tab));
}

void TabSupervisor::switchToFirstAvailableNetworkTab()
{
    if (!roomTabs.isEmpty()) {
        setCurrentWidget(roomTabs.first());
    } else if (tabServer) {
        setCurrentWidget(tabServer);
    } else {
        openTabServer();
    }
}

void TabSupervisor::openReplay(GameReplay *replay)
{
    auto *replayTab = new TabGame(this, replay);
    connect(replayTab, &TabGame::gameClosing, this, &TabSupervisor::replayLeft);
    myAddTab(replayTab);
    replayTabs.append(replayTab);
    setCurrentWidget(replayTab);
}

void TabSupervisor::replayLeft(TabGame *tab)
{
    if (tab == currentWidget()) {
        emit setMenu();
    }

    replayTabs.removeOne(tab);
}

void TabSupervisor::joinReportGame(const int gameId, const int roomId)
{
    auto *remoteClient = qobject_cast<RemoteClient *>(client);
    if (!remoteClient) {
        actShowPopup(tr("Report joins are only available on a remote server."));
        return;
    }

    auto ctx = std::make_unique<ContextJoinGame>();
    ctx->roomContext.serverContext.hostname = remoteClient->peerName();
    ctx->roomContext.serverContext.port = QString::number(remoteClient->peerPort());
    ctx->roomContext.roomId = roomId;
    ctx->gameId = gameId;
    ctx->asSpectator = true;

    auto *joinGameIntent = new IntentJoinServerGame(this, remoteClient, std::move(ctx));
    joinGameIntent->setParent(this);
    connect(joinGameIntent, &Intent::failed, this, [gameId](const QString &reason) {
        actShowPopup(tr("Could not join game %1.\n%2").arg(gameId).arg(reason));
    });

    joinGameIntent->execute();
}

TabMessage *TabSupervisor::addMessageTab(const QString &receiverName, bool focus)
{
    if (receiverName == QString::fromStdString(userInfo->name())) {
        return nullptr;
    }

    ServerInfo_User otherUser;
    bool userOnline = false;
    if (auto user = userListManager->getOnlineUser(receiverName)) {
        otherUser = ServerInfo_User(*user);
        userOnline = true;
    } else {
        otherUser.set_name(receiverName.toStdString());
    }

    TabMessage *tab;
    tab = messageTabs.value(QString::fromStdString(otherUser.name()));
    if (tab) {
        if (focus) {
            setCurrentWidget(tab);
        }
        return tab;
    }

    tab = new TabMessage(this, client, *userInfo, otherUser, userOnline);
    connect(tab, &TabMessage::talkClosing, this, &TabSupervisor::talkLeft);
    connect(tab, &TabMessage::maximizeClient, this, &TabSupervisor::maximizeMainWindow);
    connect(tab, &TabMessage::cockatriceLinkActivated, this, &TabSupervisor::cockatriceLinkActivated);
    myAddTab(tab);
    messageTabs.insert(receiverName, tab);
    if (focus) {
        setCurrentWidget(tab);
    }
    return tab;
}

void TabSupervisor::maximizeMainWindow()
{
    emit showWindowIfHidden();
}

void TabSupervisor::talkLeft(TabMessage *tab)
{
    if (tab == currentWidget()) {
        emit setMenu();
    }

    messageTabs.remove(tab->getUserName());
    removeTab(indexOf(tab));
}

QList<GameInviteOption> TabSupervisor::getGameInviteLinksForRoom(int roomId) const
{
    QList<GameInviteOption> options;
    if (isLocalGame) {
        return options;
    }

    // The inviter may be in several games of the same room (hosting one and
    // spectating another, for example). Return every game so the caller can
    // let the user choose which one to invite to.
    for (auto it = gameTabs.cbegin(); it != gameTabs.cend(); ++it) {
        TabGame *tab = it.value();
        GameMetaInfo *metaInfo = tab->getGame()->getGameMetaInfo();
        if (metaInfo->proto().room_id() != roomId) {
            continue;
        }
        // A closed game is a dead end — drop it. Started/full games stay
        // listed: an invite to them is a legitimate "come spectate" offer.
        if (metaInfo->proto().closed()) {
            continue;
        }

        const int gameId = metaInfo->gameId();
        const QString description = QString::fromStdString(metaInfo->proto().description());

        GameInviteOption option{
            .gameId = gameId,
            .label =
                description.isEmpty() ? tr("Game #%1").arg(gameId) : tr("Game #%1 — %2").arg(gameId).arg(description),
            .url = makeGameJoinLink(client->serverName(), client->serverPort(), roomId, gameId, description),
            .description = description,
            .onlyBuddies = metaInfo->proto().only_buddies(),
            .creatorName = QString::fromStdString(metaInfo->proto().creator_info().name()),
        };
        options.append(option);
    }

    return options;
}

void TabSupervisor::sendInviteToUser(const QString &userName, const QString &inviteText)
{
    TabMessage *tab = addMessageTab(userName, true);
    if (tab) {
        tab->sendInviteMessage(inviteText);
    }
}

/**
 * Creates a new deck editor tab and loads the deck into it.
 * Creates either a classic or visual deck editor tab depending on settings
 * @param deckToOpen The deck to open in the tab.
 */
void TabSupervisor::openDeckInNewTab(const LoadedDeck &deckToOpen)
{
    int type = SettingsCache::instance().deckEditor().getDefaultDeckEditorType();
    switch (type) {
        case ClassicDeckEditor:
            addDeckEditorTab(deckToOpen);
            break;
        case VisualDeckEditor:
            addVisualDeckEditorTab(deckToOpen);
            break;
        default:
            qCWarning(TabSupervisorLog) << "Unknown DeckEditorType [" << type
                                        << "]; opening ClassicDeckEditor as fallback";
            addDeckEditorTab(deckToOpen);
            break;
    }
}

/**
 * Creates a new deck editor tab
 * @param deckToOpen The deck to open in the tab.
 */
TabDeckEditor *TabSupervisor::addDeckEditorTab(const LoadedDeck &deckToOpen)
{
    auto *tab = new TabDeckEditor(this);
    tab->openDeck(deckToOpen);
    connect(tab, &AbstractTabDeckEditor::deckEditorClosing, this, &TabSupervisor::deckEditorClosed);
    connect(tab, &AbstractTabDeckEditor::openDeckEditor, this, &TabSupervisor::addDeckEditorTab);
    myAddTab(tab);
    deckEditorTabs.append(tab);
    setCurrentWidget(tab);
    return tab;
}

TabDeckEditorVisual *TabSupervisor::addVisualDeckEditorTab(const LoadedDeck &deckToOpen)
{
    auto *tab = new TabDeckEditorVisual(this);
    tab->openDeck(deckToOpen);
    connect(tab, &AbstractTabDeckEditor::deckEditorClosing, this, &TabSupervisor::deckEditorClosed);
    connect(tab, &AbstractTabDeckEditor::openDeckEditor, this, &TabSupervisor::addVisualDeckEditorTab);
    myAddTab(tab);
    deckEditorTabs.append(tab);
    setCurrentWidget(tab);
    return tab;
}

TabEdhRecMain *TabSupervisor::addEdhrecMainTab()
{
    auto *tab = new TabEdhRecMain(this);

    myAddTab(tab);
    setCurrentWidget(tab);
    return tab;
}

TabArchidekt *TabSupervisor::addArchidektTab()
{
    auto *tab = new TabArchidekt(this);

    myAddTab(tab);
    setCurrentWidget(tab);
    return tab;
}

TabVisualDatabaseDisplay *TabSupervisor::addVisualDatabaseDisplayTab()
{
    auto *tab = new TabVisualDatabaseDisplay(this);
    myAddTab(tab);
    setCurrentWidget(tab);
    return tab;
}

TabEdhRec *TabSupervisor::addEdhrecTab(const CardInfoPtr &cardToQuery, bool isCommander)
{
    auto *tab = new TabEdhRec(this);
    if (cardToQuery) {
        tab->setCard(cardToQuery, isCommander);
    }

    myAddTab(tab);
    setCurrentWidget(tab);
    return tab;
}

void TabSupervisor::deckEditorClosed(AbstractTabDeckEditor *tab)
{
    if (tab == currentWidget()) {
        emit setMenu();
    }

    deckEditorTabs.removeOne(tab);
    removeTab(indexOf(tab));
}

void TabSupervisor::tabUserEvent(bool globalEvent)
{
    auto *tab = static_cast<Tab *>(sender());
    if (tab != currentWidget()) {
        tab->setContentsChanged(true);
        setTabIcon(indexOf(tab), QPixmap("theme:icons/tab_changed"));
    }
    if (globalEvent && SettingsCache::instance().userInterface().getNotificationsEnabled()) {
        QApplication::alert(this);
    }
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
    if (tab) {
        tab->processRoomEvent(event);
    }
}

void TabSupervisor::processGameEventContainer(const GameEventContainer &cont)
{
    TabGame *tab = gameTabs.value(cont.game_id());
    if (tab) {
        tab->getGame()->getGameEventHandler()->processGameEventContainer(cont, qobject_cast<AbstractClient *>(sender()),
                                                                         {});
    } else {
        qCInfo(TabSupervisorLog) << "gameEvent: invalid gameId" << cont.game_id();
    }
}

void TabSupervisor::processUserMessageEvent(const Event_UserMessage &event)
{
    QString senderName = QString::fromStdString(event.sender_name());
    TabMessage *tab = messageTabs.value(senderName);
    if (!tab) {
        tab = messageTabs.value(QString::fromStdString(event.receiver_name()));
    }
    if (!tab) {
        const ServerInfo_User *onlineUserInfo = userListManager->getOnlineUser(senderName);
        if (onlineUserInfo) {
            auto userLevel = UserLevelFlags(onlineUserInfo->user_level());
            if (SettingsCache::instance().chat().getIgnoreUnregisteredUserMessages() &&
                !userLevel.testFlag(ServerInfo_User::IsRegistered)) {
                // Flags are additive, so reg/mod/admin are all IsRegistered
                return;
            } else if (SettingsCache::instance().chat().getIgnoreNonBuddyUserMessages() &&
                       !userListManager->isUserBuddy(senderName) && !userLevel.testFlag(ServerInfo_User::IsModerator) &&
                       !userLevel.testFlag(ServerInfo_User::IsAdmin)) {
                // Ignore private messages from non-buddies
                // Moderator/Admin messages are exempt to ensure warnings reach users
                return;
            }
        }
        tab = addMessageTab(QString::fromStdString(event.sender_name()), false);
    }
    if (!tab) {
        return;
    }
    tab->processUserMessageEvent(event);
}

void TabSupervisor::actShowPopup(const QString &message)
{
    qCDebug(TabSupervisorLog) << "ACT SHOW POPUP";
    if (trayIcon && (QApplication::activeWindow() == nullptr || QApplication::focusWidget() == nullptr)) {
        qCDebug(TabSupervisorLog) << "LAUNCHING POPUP";
        // disconnect(trayIcon, SIGNAL(messageClicked()), nullptr, nullptr);
        trayIcon->showMessage(message, tr("Click to view"));
        // connect(trayIcon, SIGNAL(messageClicked()), chatView, SLOT(actMessageClicked()));
    }
}

void TabSupervisor::processUserLeft(const QString &userName)
{
    TabMessage *tab = messageTabs.value(userName);
    if (tab) {
        tab->processUserLeft();
    }
}

void TabSupervisor::processUserJoined(const ServerInfo_User &userInfoJoined)
{
    QString userName = QString::fromStdString(userInfoJoined.name());
    if (userListManager->isUserBuddy(userName)) {
        if (auto *tab = getTabAccount()) {
            if (tab != currentWidget()) {
                tab->setContentsChanged(true);
                QIcon avatarIcon = UserLevelPixmapGenerator::generateIcon(
                    13, (UserLevelFlags)userInfoJoined.user_level(), userInfoJoined.pawn_colors(), true,
                    QString::fromStdString(userInfoJoined.privlevel()));
                setTabIcon(indexOf(tab), avatarIcon);
            }
        }

        if (SettingsCache::instance().userInterface().getBuddyConnectNotificationsEnabled()) {
            QApplication::alert(this);
            this->actShowPopup(tr("Your buddy %1 has signed on!").arg(userName));
        }
    }

    TabMessage *tab = messageTabs.value(userName);
    if (tab) {
        tab->processUserJoined(userInfoJoined);
    }
}

void TabSupervisor::updateCurrent(int index)
{
    if (index != -1) {
        auto *tab = static_cast<Tab *>(widget(index));
        if (tab->getContentsChanged()) {
            setTabIcon(index, QIcon());
            tab->setContentsChanged(false);
        }
        emit setMenu(static_cast<Tab *>(widget(index))->getTabMenus());
        tab->tabActivated();
    } else {
        emit setMenu();
    }
}

/**
 * Determine if a user is a moderator/administrator
 * By seeing if they have the admin tab open & unlocked
 * @return if the admin tab is open & unlocked
 */
bool TabSupervisor::getAdminLocked() const
{
    if (!tabAdmin) {
        return true;
    }
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
            if (!QString::fromStdString(event.warning_reason()).simplified().isEmpty()) {
                QMessageBox::warning(this, tr("Warned"),
                                     tr("You have received a warning due to %1.\nPlease refrain from engaging in this "
                                        "activity or further actions may be taken against you. If you have any "
                                        "questions, please private message a moderator.")
                                         .arg(QString::fromStdString(event.warning_reason()).simplified()));
            }
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
        case Event_NotifyUser::REPORT_RESOLVED: {
            QString title = QString::fromStdString(event.custom_title()).simplified();
            QString content = QString::fromStdString(event.custom_content()).trimmed();
            if (!title.isEmpty() && !content.isEmpty()) {
                actShowPopup(title + "\n" + content);
                QApplication::alert(this);
            }
            break;
        }
        case Event_NotifyUser::REPORT_COMMENT: {
            QString title = QString::fromStdString(event.custom_title()).simplified();
            QString content = QString::fromStdString(event.custom_content()).trimmed();
            if (!title.isEmpty() && !content.isEmpty()) {
                actShowPopup(title + "\n" + content);
                QApplication::alert(this);
            }
            break;
        }
        default:;
    }
}

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
