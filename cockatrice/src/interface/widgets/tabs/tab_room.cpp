#include "tab_room.h"

#include "../../../client/settings/cache_settings.h"
#include "../../../client/settings/shortcuts_settings.h"
#include "../../pixel_map_generator.h"
#include "../interface/widgets/dialogs/dlg_settings.h"
#include "../interface/widgets/server/chat_view/chat_view.h"
#include "../interface/widgets/server/game_link.h"
#include "../interface/widgets/server/game_selector.h"
#include "../interface/widgets/server/user/user_list_manager.h"
#include "../interface/widgets/server/user/user_list_panel_widget.h"
#include "../interface/widgets/server/user/user_list_widget.h"
#include "../main.h"
#include "../server/game_type_map.h"
#include "../server/user/user_list_proxy.h"
#include "../utility/completer_utils.h"
#include "../utility/line_edit_completer.h"
#include "card/card_completer_proxy_model.h"
#include "card/card_search_model.h"
#include "card_database_display_model.h"
#include "card_database_model.h"
#include "libcockatrice/protocol/pb/response.pb.h"
#include "libcockatrice/protocol/pb/room_event.pb.h"
#include "libcockatrice/protocol/pb/serverinfo_gametype.pb.h"
#include "libcockatrice/protocol/pb/serverinfo_user.pb.h"
#include "tab_supervisor.h"
#include "user_level.h"

#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QColor>
#include <QCompleter>
#include <QDateTime>
#include <QFlag>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QRegularExpression>
#include <QSplitter>
#include <QStackedWidget>
#include <QStringListModel>
#include <QStringLiteral>
#include <QSystemTrayIcon>
#include <QTabBar>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidget>
#include <functional>
#include <libcockatrice/card/database/card_database_manager.h>
#include <libcockatrice/network/client/abstract/abstract_client.h>
#include <libcockatrice/protocol/get_pb_extension.h>
#include <libcockatrice/protocol/pb/event_join_room.pb.h>
#include <libcockatrice/protocol/pb/event_leave_room.pb.h>
#include <libcockatrice/protocol/pb/event_list_games.pb.h>
#include <libcockatrice/protocol/pb/event_remove_messages.pb.h>
#include <libcockatrice/protocol/pb/event_room_channel_say.pb.h>
#include <libcockatrice/protocol/pb/event_room_say.pb.h>
#include <libcockatrice/protocol/pb/room_commands.pb.h>
#include <libcockatrice/protocol/pb/serverinfo_room.pb.h>
#include <libcockatrice/protocol/pending_command.h>
#include <libcockatrice/settings/chat_settings.h>
#include <libcockatrice/utility/string_limits.h>
#include <qnamespace.h>
#include <string>

namespace
{
const QString MAIN_CHANNEL;

/**
 * @brief Privilege rank mirroring Server_Room::userPrivilegeLevel.
 *
 * Higher = more privileged. Used to evaluate channel write access locally so
 * the input can reflect read-only channels; the server re-checks on send.
 */
int userPrivilegeRank(uint32_t userLevel)
{
    if (userLevel & ServerInfo_User::IsDeveloper) {
        return static_cast<int>(ServerInfo_RoomChannel::Developer);
    }
    if (userLevel & ServerInfo_User::IsAdmin) {
        return static_cast<int>(ServerInfo_RoomChannel::Admin);
    }
    if (userLevel & ServerInfo_User::IsModerator) {
        return static_cast<int>(ServerInfo_RoomChannel::Moderator);
    }
    if (userLevel & ServerInfo_User::IsJudge) {
        return static_cast<int>(ServerInfo_RoomChannel::Judge);
    }
    if (userLevel & ServerInfo_User::IsRegistered) {
        return static_cast<int>(ServerInfo_RoomChannel::Registered);
    }
    return static_cast<int>(ServerInfo_RoomChannel::Public);
}
} // namespace

TabRoom::TabRoom(TabSupervisor *_tabSupervisor,
                 AbstractClient *_client,
                 ServerInfo_User *_ownUser,
                 const ServerInfo_Room &info)
    : Tab(_tabSupervisor), client(_client), roomId(info.room_id()), roomName(QString::fromStdString(info.name())),
      ownUser(_ownUser), userListProxy(_tabSupervisor->getUserListManager()), activeChannelId(MAIN_CHANNEL),
      chatCollapsed(false)
{
    const int gameTypeListSize = info.gametype_list_size();
    for (int i = 0; i < gameTypeListSize; ++i) {
        gameTypes.insert(info.gametype_list(i).game_type_id(),
                         QString::fromStdString(info.gametype_list(i).description()));
    }

    QMap<int, GameTypeMap> tempMap;
    tempMap.insert(info.room_id(), gameTypes);
    gameSelector = new GameSelector(client, tabSupervisor, this, QMap<int, QString>(), tempMap, true, true);

    userListPanel = new UserListPanelWidget(tabSupervisor, client, this);
    userListPanel->bind(tabSupervisor->getUserListManager());
    userList = userListPanel->getUserList();
    connect(userListPanel, &UserListPanelWidget::openMessageDialog, this, &TabRoom::openMessageDialog);

    const auto gameInviteLinkProvider = [this]() { return tabSupervisor->getGameInviteLinksForRoom(roomId); };
    userList->setGameInviteLinkProvider(gameInviteLinkProvider);

    chatViewStack = new QStackedWidget;
    channelTabBar = new QTabBar;
    channelTabBar->setShape(QTabBar::RoundedSouth);
    channelTabBar->setExpanding(true);
    channelTabBar->setUsesScrollButtons(true);

    channelViews.insert(MAIN_CHANNEL, makeChatView());
    channelIds << MAIN_CHANNEL;
    channelDisplayNames.insert(MAIN_CHANNEL, tr("Main"));
    channelUnreadCounts.insert(MAIN_CHANNEL, 0);
    channelMentioned.insert(MAIN_CHANNEL, false);
    channelWritable.insert(MAIN_CHANNEL, true);
    channelTabBar->addTab(channelDisplayNames.value(MAIN_CHANNEL));
    chatViewStack->addWidget(channelViews.value(MAIN_CHANNEL));

    const uint32_t ownLevel = ownUser != nullptr ? ownUser->user_level() : 0;
    const int ownRank = userPrivilegeRank(ownLevel);

    const int channelCount = info.channel_list_size();
    for (int i = 0; i < channelCount; ++i) {
        const ServerInfo_RoomChannel &channel = info.channel_list(i);
        const QString channelId = QString::fromStdString(channel.id());
        if (channelId.isEmpty()) {
            continue;
        }
        QString displayName = QString::fromStdString(channel.display_name());
        if (displayName.isEmpty()) {
            displayName = channelId;
        }
        const int writeLevel = channel.has_write_access_level() ? static_cast<int>(channel.write_access_level())
                                                                : static_cast<int>(channel.access_level());
        ChatView *channelView = makeChatView();
        const QString banner = QString::fromStdString(channel.banner_message());
        if (!banner.isEmpty()) {
            channelView->appendMessage(banner);
        }
        channelViews.insert(channelId, channelView);
        channelIds << channelId;
        channelDisplayNames.insert(channelId, displayName);
        channelUnreadCounts.insert(channelId, 0);
        channelMentioned.insert(channelId, false);
        channelWritable.insert(channelId, ownRank >= writeLevel);
        channelTabBar->addTab(displayName);
        chatViewStack->addWidget(channelViews.value(channelId));
    }

    chatView = channelViews.value(MAIN_CHANNEL);
    chatViewStack->setCurrentWidget(chatView);
    channelTabBar->setCurrentIndex(0);
    connect(channelTabBar, &QTabBar::currentChanged, this, &TabRoom::onChannelChanged);
    connect(channelTabBar, &QTabBar::tabBarClicked, this, &TabRoom::onChannelTabClicked);
    connect(&SettingsCache::instance().chat(), &ChatSettings::chatMentionCompleterChanged, this,
            &TabRoom::actCompleterChanged);
    sayLabel = new QLabel;
    sayEdit = new LineEditCompleter;
    sayEdit->setMaxLength(MAX_TEXT_LENGTH);
    sayLabel->setBuddy(sayEdit);
    connect(sayEdit, &LineEditCompleter::returnPressed, this, &TabRoom::sendMessage);
    updateSayEnabled();

    auto *chatSettingsMenu = new QMenu(this);

    aClearChat = chatSettingsMenu->addAction(QString());
    connect(aClearChat, &QAction::triggered, this, &TabRoom::actClearChat);

    chatSettingsMenu->addSeparator();

    aOpenChatSettings = chatSettingsMenu->addAction(QString());
    connect(aOpenChatSettings, &QAction::triggered, this, &TabRoom::actOpenChatSettings);

    auto *chatSettingsButton = new QToolButton;
    chatSettingsButton->setIcon(themePixmap(QStringLiteral("icons/settings")));
    chatSettingsButton->setMenu(chatSettingsMenu);
    chatSettingsButton->setPopupMode(QToolButton::InstantPopup);

    auto *sayHbox = new QHBoxLayout;
    sayHbox->addWidget(sayLabel);
    sayHbox->addWidget(sayEdit);
    sayHbox->addWidget(chatSettingsButton);

    sayWidget = new QWidget;
    sayWidget->setLayout(sayHbox);

    auto *chatVbox = new QVBoxLayout;
    chatVbox->addWidget(chatViewStack);
    chatVbox->addWidget(sayWidget);
    chatVbox->addWidget(channelTabBar);

    chatGroupBox = new QGroupBox;
    chatGroupBox->setLayout(chatVbox);

    chatSplitter = new QSplitter(Qt::Vertical);
    chatSplitter->addWidget(gameSelector);
    chatSplitter->addWidget(chatGroupBox);

    auto *hbox = new QHBoxLayout;
    hbox->addWidget(chatSplitter, 3);
    hbox->addWidget(userListPanel, 1);

    aLeaveRoom = new QAction(this);
    connect(aLeaveRoom, &QAction::triggered, this, &TabRoom::closeRequest);

    roomMenu = new QMenu(this);
    roomMenu->addAction(aLeaveRoom);
    addTabMenu(roomMenu);

    const int userListSize = info.user_list_size();
    for (int i = 0; i < userListSize; ++i) {
        autocompleteUserList.append("@" + QString::fromStdString(info.user_list(i).name()));
    }

    const int gameListSize = info.game_list_size();
    for (int i = 0; i < gameListSize; ++i) {
        gameSelector->processGameInfo(info.game_list(i));
    }

    mentionModel = new QStringListModel(autocompleteUserList, sayEdit);
    mentionCompleter = createMentionCompleter(mentionModel, sayEdit);
    sayEdit->addCompleter(mentionCompleter, CompleterTrigger::Mention);

    auto *cardDatabaseModel = new CardDatabaseModel(CardDatabaseManager::getInstance(), false, sayEdit);
    auto *displayModel = new CardDatabaseDisplayModel(sayEdit);
    displayModel->setSourceModel(cardDatabaseModel);
    const CardCompleterSetup cardSetup = createCardCompleter(displayModel, sayEdit);
    sayEdit->addCompleter(cardSetup.completer, CompleterTrigger::Card);

    connect(sayEdit, &LineEditCompleter::cardPartialChanged, this, [this, cardSetup](const QString &text) {
        cardSetup.searchModel->updateSearchResults(text);
        cardSetup.proxyModel->setFilterRegularExpression(
            QRegularExpression(QRegularExpression::escape(text), QRegularExpression::CaseInsensitiveOption));
        if (sayEdit->hasFocus()) {
            cardSetup.completer->complete();
        }
    });

    actCompleterChanged();

    connect(&SettingsCache::instance().shortcuts(), &ShortcutsSettings::shortCutChanged, this,
            &TabRoom::refreshShortcuts);
    refreshShortcuts();

    retranslateUi();

    auto *mainWidget = new QWidget(this);
    mainWidget->setLayout(hbox);
    setCentralWidget(mainWidget);
}

void TabRoom::retranslateUi()
{
    gameSelector->retranslateUi();
    for (auto it = channelViews.constBegin(); it != channelViews.constEnd(); ++it) {
        it.value()->retranslateUi();
    }
    userListPanel->retranslateUi();
    sayLabel->setText(tr("&Say:"));
    chatGroupBox->setTitle(tr("Chat"));
    roomMenu->setTitle(tr("&Room"));
    aLeaveRoom->setText(tr("&Leave room"));
    aClearChat->setText(tr("&Clear chat"));
    aOpenChatSettings->setText(tr("Chat Settings..."));
    channelDisplayNames.insert(MAIN_CHANNEL, tr("Main"));
    for (const QString &channelId : channelIds) {
        updateChannelTab(channelId);
    }
    updateSayEnabled();
}

void TabRoom::focusTab()
{
    activateWindow();
    tabSupervisor->setCurrentIndex(tabSupervisor->indexOf(this));
    emit maximizeClient();
}

void TabRoom::actShowMentionPopup(const QString &sender)
{
    this->actShowPopup(sender + tr(" mentioned you."));
}

void TabRoom::actShowPopup(const QString &message)
{
    if (trayIcon && (tabSupervisor->currentIndex() != tabSupervisor->indexOf(this) ||
                     QApplication::activeWindow() == nullptr || QApplication::focusWidget() == nullptr)) {
        disconnect(trayIcon, &QSystemTrayIcon::messageClicked, nullptr, nullptr);
        trayIcon->showMessage(message, tr("Click to view"));
        connect(trayIcon, &QSystemTrayIcon::messageClicked, chatView, &ChatView::messageClickedSignal);
    }
}

void TabRoom::closeEvent(QCloseEvent *event)
{
    sendRoomCommand(prepareRoomCommand(Command_LeaveRoom()));
    emit roomClosing(this);
    event->accept();
}

void TabRoom::tabActivated()
{
    if (!sayEdit->hasFocus()) {
        sayEdit->setFocus();
    }
    if (!chatCollapsed) {
        channelUnreadCounts[activeChannelId] = 0;
        channelMentioned[activeChannelId] = false;
        updateChannelTab(activeChannelId);
    }
}

QString TabRoom::sanitizeHtml(QString dirty) const
{
    return dirty.replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;");
}

void TabRoom::updateSayEnabled()
{
    const bool writable = channelWritable.value(activeChannelId, true);
    sayLabel->setEnabled(writable);
    sayEdit->setEnabled(writable);
    sayEdit->setPlaceholderText(writable ? QString() : tr("This channel is read-only."));
}

void TabRoom::sendMessage()
{
    if (!channelWritable.value(activeChannelId, true)) {
        return;
    }
    if (sayEdit->text().isEmpty()) {
        return;
    } else if (sayEdit->hasVisibleCompleterPopup()) {
        sayEdit->hideCompleterPopups();
        return;
    } else {
        if (activeChannelId.isEmpty()) {
            Command_RoomSay cmd;
            cmd.set_message(sayEdit->text().toStdString());

            PendingCommand *pend = prepareRoomCommand(cmd);
            connect(pend, &PendingCommand::finished, this, &TabRoom::sayFinished);
            sendRoomCommand(pend);
        } else {
            Command_RoomChannelSay cmd;
            cmd.set_channel_id(activeChannelId.toStdString());
            cmd.set_message(sayEdit->text().toStdString());

            PendingCommand *pend = prepareRoomCommand(cmd);
            connect(pend, &PendingCommand::finished, this, &TabRoom::sayFinished);
            sendRoomCommand(pend);
        }
        sayEdit->clear();
    }
}

void TabRoom::onChannelChanged(int index)
{
    if (index < 0 || index >= channelIds.size()) {
        return;
    }
    activeChannelId = channelIds.at(index);
    ChatView *view = channelViews.value(activeChannelId);
    if (view) {
        chatView = view;
        chatViewStack->setCurrentWidget(view);
    }
    // Viewing a channel clears its unread state
    channelUnreadCounts[activeChannelId] = 0;
    channelMentioned[activeChannelId] = false;
    updateChannelTab(activeChannelId);
    updateSayEnabled();
    if (chatCollapsed) {
        setChatCollapsed(false);
    }
}

void TabRoom::onChannelTabClicked(int index)
{
    if (index < 0 || index >= channelIds.size()) {
        return;
    }
    // Clicking the active tab toggles collapse; clicking another tab switches to it
    if (channelTabBar->currentIndex() == index && channelIds.at(index) == activeChannelId) {
        setChatCollapsed(!chatCollapsed);
    }
}

void TabRoom::setChatCollapsed(bool collapsed)
{
    chatCollapsed = collapsed;
    chatViewStack->setVisible(!collapsed);
    sayWidget->setVisible(!collapsed);
    if (collapsed) {
        // Minimize the chat section of the splitter down to just the tab bar
        savedSplitterSizes = chatSplitter->sizes();
        chatSplitter->setSizes({1, 0});
    } else {
        // Expanding onto the active channel marks it read
        channelUnreadCounts[activeChannelId] = 0;
        channelMentioned[activeChannelId] = false;
        updateChannelTab(activeChannelId);
        chatViewStack->setCurrentWidget(chatView);
        if (!savedSplitterSizes.isEmpty()) {
            chatSplitter->setSizes(savedSplitterSizes);
        }
    }
}

QColor TabRoom::parseColorHex(const QString &hex, const QColor &fallback)
{
#if (QT_VERSION >= QT_VERSION_CHECK(6, 4, 0))
    QColor color = QColor::fromString("#" + hex);
#else
    QColor color;
    color.setNamedColor("#" + hex);
#endif
    return color.isValid() ? color : fallback;
}

void TabRoom::updateChannelTab(const QString &channelId)
{
    int index = channelIds.indexOf(channelId);
    if (index < 0) {
        return;
    }
    QString baseName = channelDisplayNames.value(channelId, channelId.isEmpty() ? tr("Main") : channelId);
    int unread = channelUnreadCounts.value(channelId, 0);
    bool mentioned = channelMentioned.value(channelId, false);

    QString label = baseName;
    if (unread > 0) {
        label += tr(" (%1)").arg(unread);
    }
    channelTabBar->setTabText(index, label);

    if (mentioned) {
        channelTabBar->setTabTextColor(
            index, parseColorHex(SettingsCache::instance().chat().getChannelMentionColor(), QColor(194, 31, 47)));
    } else if (unread > 0) {
        channelTabBar->setTabTextColor(
            index, parseColorHex(SettingsCache::instance().chat().getChannelUnreadColor(), QColor(26, 123, 0)));
    } else {
        channelTabBar->setTabTextColor(index, QColor());
    }
}

bool TabRoom::messageContainsMention(const QString &message) const
{
    if (ownUser == nullptr) {
        return false;
    }
    QString ownName = QString::fromStdString(ownUser->name());
    if (ownName.isEmpty()) {
        return false;
    }
    if (message.contains("@" + ownName, Qt::CaseInsensitive)) {
        return true;
    }
    if (message.contains("@all", Qt::CaseInsensitive)) {
        return true;
    }
    const QStringList words = SettingsCache::instance().chat().getHighlightWords().split(' ', Qt::SkipEmptyParts);
    for (const QString &word : words) {
        if (!word.isEmpty() && message.contains(word, Qt::CaseInsensitive)) {
            return true;
        }
    }
    return false;
}

void TabRoom::markChannelMessage(const QString &channelId, const QString &message)
{
    bool roomVisible = (tabSupervisor->currentWidget() == this);
    bool viewing = (channelId == activeChannelId) && !chatCollapsed && roomVisible;
    if (viewing) {
        return;
    }
    channelUnreadCounts[channelId] = channelUnreadCounts.value(channelId, 0) + 1;
    if (messageContainsMention(message)) {
        channelMentioned[channelId] = true;
    }
    updateChannelTab(channelId);
}

ChatView *TabRoom::makeChatView()
{
    ChatView *view = new ChatView(tabSupervisor, nullptr, true, this);
    connect(view, &ChatView::showMentionPopup, this, &TabRoom::actShowMentionPopup);
    connect(view, &ChatView::messageClickedSignal, this, &TabRoom::focusTab);
    connect(view, &ChatView::openMessageDialog, this, &TabRoom::openMessageDialog);
    connect(view, &ChatView::cockatriceLinkActivated, this, &TabRoom::cockatriceLinkActivated);
    connect(view, &ChatView::showCardInfoPopup, this, &TabRoom::showCardInfoPopup);
    connect(view, &ChatView::deleteCardInfoPopup, this, &TabRoom::deleteCardInfoPopup);
    connect(view, &ChatView::addMentionTag, this, &TabRoom::addMentionTag);
    return view;
}

void TabRoom::sayFinished(const Response &response)
{
    if (response.response_code() == Response::RespChatFlood) {
        chatView->appendMessage(tr("You are flooding the chat. Please wait a couple of seconds."));
    } else if (response.response_code() == Response::RespFunctionNotAllowed ||
               response.response_code() == Response::RespInvalidCommand) {
        chatView->appendMessage(tr("Your message could not be sent: the channel is not available or you are not "
                                   "allowed to talk there."));
    }
}

void TabRoom::actClearChat()
{
    chatView->clearChat();
}

void TabRoom::actOpenChatSettings()
{
    DlgSettings settings(this);
    settings.setTab(DlgSettings::MessagesPage);
    settings.exec();
}

void TabRoom::actCompleterChanged()
{
    SettingsCache::instance().chat().getChatMentionCompleter() ? mentionCompleter->setCompletionRole(2)
                                                               : mentionCompleter->setCompletionRole(1);
}

void TabRoom::processRoomEvent(const RoomEvent &event)
{
    switch (static_cast<RoomEvent::RoomEventType>(getPbExtension(event))) {
        case RoomEvent::LIST_GAMES:
            processListGamesEvent(event.GetExtension(Event_ListGames::ext));
            break;
        case RoomEvent::JOIN_ROOM:
            processJoinRoomEvent(event.GetExtension(Event_JoinRoom::ext));
            break;
        case RoomEvent::LEAVE_ROOM:
            processLeaveRoomEvent(event.GetExtension(Event_LeaveRoom::ext));
            break;
        case RoomEvent::ROOM_SAY:
            processRoomSayEvent(event.GetExtension(Event_RoomSay::ext));
            break;
        case RoomEvent::ROOM_CHANNEL_SAY:
            processRoomChannelSayEvent(event.GetExtension(Event_RoomChannelSay::ext));
            break;
        case RoomEvent::REMOVE_MESSAGES:
            processRemoveMessagesEvent(event.GetExtension(Event_RemoveMessages::ext));
            break;
        default:;
    }
}

void TabRoom::processListGamesEvent(const Event_ListGames &event)
{
    const int gameListSize = event.game_list_size();
    for (int i = 0; i < gameListSize; ++i) {
        gameSelector->processGameInfo(event.game_list(i));
    }
    emit gameListUpdated();
}

void TabRoom::processJoinRoomEvent(const Event_JoinRoom &event)
{
    QString mention = "@" + QString::fromStdString(event.user_info().name());
    if (!autocompleteUserList.contains(mention)) {
        autocompleteUserList << mention;
        mentionModel->setStringList(autocompleteUserList);
    }
}

void TabRoom::processLeaveRoomEvent(const Event_LeaveRoom &event)
{
    QString mention = "@" + QString::fromStdString(event.name());
    autocompleteUserList.removeOne(mention);
    mentionModel->setStringList(autocompleteUserList);
}

void TabRoom::processRoomSayEvent(const Event_RoomSay &event)
{
    QString senderName = QString::fromStdString(event.name());
    QString message = QString::fromStdString(event.message());

    if (userListProxy->isUserIgnored(senderName)) {
        return;
    }

    UserListTWI *twi = userList->getUsers().value(senderName);
    ServerInfo_User userInfo = {};
    if (twi) {
        userInfo = twi->getUserInfo();
        if (SettingsCache::instance().chat().getIgnoreUnregisteredUsers() &&
            !UserLevelFlags(userInfo.user_level()).testFlag(ServerInfo_User::IsRegistered)) {
            return;
        }
    }

    if (event.message_type() == Event_RoomSay::ChatHistory && !SettingsCache::instance().chat().getRoomHistory()) {
        return;
    }

    if (event.message_type() == Event_RoomSay::ChatHistory) {
        message =
            "[" +
            QString(QDateTime::fromMSecsSinceEpoch(event.time_of()).toLocalTime().toString("d MMM yyyy HH:mm:ss")) +
            "] " + message;
    }

    ChatView *mainView = channelViews.value(MAIN_CHANNEL);
    if (mainView == nullptr) {
        return;
    }

    mainView->appendMessage(message, event.message_type(), userInfo, true);
    if (event.message_type() != Event_RoomSay::ChatHistory) {
        markChannelMessage(MAIN_CHANNEL, message);
    }
    emit userEvent(false);
}

void TabRoom::processRoomChannelSayEvent(const Event_RoomChannelSay &event)
{
    const QString channelId = QString::fromStdString(event.channel_id());
    ChatView *view = channelViews.value(channelId);
    if (view == nullptr) {
        return;
    }

    QString senderName = QString::fromStdString(event.name());
    QString message = QString::fromStdString(event.message());

    if (userListProxy->isUserIgnored(senderName)) {
        return;
    }

    UserListTWI *twi = userList->getUsers().value(senderName);
    ServerInfo_User userInfo = {};
    if (twi) {
        userInfo = twi->getUserInfo();
        if (SettingsCache::instance().chat().getIgnoreUnregisteredUsers() &&
            !UserLevelFlags(userInfo.user_level()).testFlag(ServerInfo_User::IsRegistered)) {
            return;
        }
    }

    if (senderName.isEmpty()) {
        if (!SettingsCache::instance().chat().getRoomHistory()) {
            return;
        }
        message =
            "[" +
            QString(QDateTime::fromMSecsSinceEpoch(event.time_of()).toLocalTime().toString("d MMM yyyy HH:mm:ss")) +
            "] " + message;
    }

    view->appendMessage(message, {}, userInfo, true);
    if (!senderName.isEmpty()) {
        // Empty senders mark replayed history, which must not count as unread
        markChannelMessage(channelId, message);
    }
    emit userEvent(false);
}

void TabRoom::processRemoveMessagesEvent(const Event_RemoveMessages &event)
{
    QString userName = QString::fromStdString(event.name());
    int amount = event.amount();
    for (ChatView *view : channelViews) {
        view->redactMessages(userName, amount);
    }
}

void TabRoom::refreshShortcuts()
{
    aClearChat->setShortcuts(SettingsCache::instance().shortcuts().getShortcut("tab_room/aClearChat"));
}

void TabRoom::addMentionTag(QString mentionTag)
{
    sayEdit->insert(mentionTag + " ");
    sayEdit->setFocus();
}

PendingCommand *TabRoom::prepareRoomCommand(const ::google::protobuf::Message &cmd)
{
    return client->prepareRoomCommand(cmd, roomId);
}

void TabRoom::sendRoomCommand(PendingCommand *pend)
{
    client->sendCommand(pend);
}
