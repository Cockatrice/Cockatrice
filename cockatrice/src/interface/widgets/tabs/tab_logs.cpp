#include "tab_logs.h"

#include "../interface/widgets/utility/custom_line_edit.h"
#include "libcockatrice/protocol/pb/response.pb.h"
#include "libcockatrice/protocol/pb/serverinfo_chat_message.pb.h"

#include <QCheckBox>
#include <QDockWidget>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QList>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QScrollBar>
#include <QSharedPointer>
#include <QSizePolicy>
#include <QSpinBox>
#include <QTabWidget>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>
#include <QWidget>
#include <QtGlobal>
#include <libcockatrice/network/client/abstract/abstract_client.h>
#include <libcockatrice/protocol/pb/moderator_commands.pb.h>
#include <libcockatrice/protocol/pb/response_viewlog_history.pb.h>
#include <libcockatrice/protocol/pending_command.h>
#include <libcockatrice/utility/string_limits.h>
#include <qnamespace.h>
#include <string>

class TabSupervisor;

namespace
{
/** @brief Identifies the newest row of a page well enough to recognise a repeated page. */
QString pageStartKey(const Response_ViewLogHistory &response)
{
    const ServerInfo_ChatMessage &first = response.log_message(0);
    return QString::fromStdString(first.time()) + QChar(0x1f) + QString::fromStdString(first.sender_name()) +
           QChar(0x1f) + QString::fromStdString(first.message());
}
} // namespace

TabLog::TabLog(TabSupervisor *_tabSupervisor, AbstractClient *_client, bool _canUseDeveloperCommands)
    : Tab(_tabSupervisor), client(_client), canUseDeveloperCommands(_canUseDeveloperCommands)
{
    roomTable = new QTableWidget();
    roomTable->setColumnCount(7);
    roomTable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    roomTable->setHorizontalHeaderLabels(
        QString(tr("Time;SenderName;SenderIP;Message;TargetID;TargetName;Channel")).split(";"));

    gameTable = new QTableWidget();
    gameTable->setColumnCount(6);
    gameTable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    gameTable->setHorizontalHeaderLabels(
        QString(tr("Time;SenderName;SenderIP;Message;TargetID;TargetName")).split(";"));

    chatTable = new QTableWidget();
    chatTable->setColumnCount(6);
    chatTable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    chatTable->setHorizontalHeaderLabels(
        QString(tr("Time;SenderName;SenderIP;Message;TargetID;TargetName")).split(";"));

    tabManager = new QTabWidget();
    tabManager->addTab(roomTable, tr("Room Logs"));
    tabManager->addTab(gameTable, tr("Game Logs"));
    tabManager->addTab(chatTable, tr("Chat Logs"));
    setCentralWidget(tabManager);

    connect(roomTable->verticalScrollBar(), &QScrollBar::valueChanged, this, &TabLog::checkFetchMore);
    connect(gameTable->verticalScrollBar(), &QScrollBar::valueChanged, this, &TabLog::checkFetchMore);
    connect(chatTable->verticalScrollBar(), &QScrollBar::valueChanged, this, &TabLog::checkFetchMore);

    createDock();
    restartLayout();
    clearClicked();
    retranslateUi();
}

TabLog::~TabLog()
{
}

void TabLog::retranslateUi()
{
}

void TabLog::getClicked()
{
    // Empty filters are a valid browse: room/game history going back from
    // today, paginated. Only require a date range + location, both of which
    // get sensible defaults below so a bare "Get" just works.
    if (!lastHour->isChecked() && !today->isChecked() && !pastDays->isChecked()) {
        pastDays->setChecked(true);
        pastXDays->setValue(20);
    }

    if (pastDays->isChecked() && pastXDays->value() == 0) {
        QMessageBox::critical(this, tr("Error"), tr("You have to select a valid number of days to locate."));
        return;
    }

    if (!mainRoom->isChecked() && !gameRoom->isChecked() && !privateChat->isChecked()) {
        mainRoom->setChecked(true);
        gameRoom->setChecked(true);
        if (!canUseDeveloperCommands) {
            privateChat->setChecked(true);
        }
    }

    if (maximumResults->value() == 0) {
        maximumResults->setValue(200);
    }
    pageSize = qBound(1, maximumResults->value(), 1000);

    // Build the command once, here, and let every page copy it: rebuilding it from the
    // widgets per page would run page 2+ against filters edited after Get was clicked,
    // appending rows selected by a different query under the first page's results.
    currentQuery.Clear();
    currentQuery.set_user_name(findUsername->text().toStdString());
    currentQuery.set_ip_address(findIPAddress->text().toStdString());
    currentQuery.set_game_name(findGameName->text().toStdString());
    currentQuery.set_game_id(findGameID->text().toStdString());
    currentQuery.set_message(findMessage->text().toStdString());
    if (mainRoom->isChecked()) {
        currentQuery.add_log_location("room");
    }
    if (gameRoom->isChecked()) {
        currentQuery.add_log_location("game");
    }
    if (privateChat->isChecked()) {
        currentQuery.add_log_location("chat");
    }
    currentQuery.set_date_range(dateRangeForSelection());
    currentQuery.set_maximum_results(pageSize);
    hasCurrentQuery = true;
    snapshotBefore.clear();
    lastPageStartKey.clear();

    // Fresh search: reset pagination state and clear previous pages.
    currentOffset = 0;
    roomCounter = 0;
    gameCounter = 0;
    chatCounter = 0;
    roomTable->setRowCount(0);
    gameTable->setRowCount(0);
    chatTable->setRowCount(0);

    fetchPage(0);
}

int TabLog::dateRangeForSelection() const
{
    if (lastHour->isChecked()) {
        return 1;
    }
    if (today->isChecked()) {
        return 24;
    }
    if (pastDays->isChecked()) {
        return pastXDays->value() * 24;
    }
    return 0;
}

void TabLog::fetchPage(int offset)
{
    if (isFetching || !hasCurrentQuery) {
        return;
    }

    Command_ViewLogHistory cmd = currentQuery;
    cmd.set_offset(offset);
    if (!snapshotBefore.isEmpty()) {
        cmd.set_before_time(snapshotBefore.toStdString());
    }

    PendingCommand *pend;
    if (canUseDeveloperCommands) {
        // Developers query logs through the developer command family.
        pend = client->prepareDeveloperCommand(cmd);
    } else {
        pend = client->prepareModeratorCommand(cmd);
    }

    isFetching = true;
    getButton->setEnabled(false);

    connect(pend, &PendingCommand::finished, this, &TabLog::viewLogHistory_processResponse);
    client->sendCommand(pend);
}

void TabLog::checkFetchMore()
{
    if (isFetching || !hasMoreResults) {
        return;
    }

    // Only the table the moderator is looking at may pull the next page. Checking all
    // three would make an empty or hidden table nearBottom by definition (maximum() and
    // value() are both zero), so a room-only browse chained page after page without any
    // scrolling - pulling the whole range with a growing OFFSET scan per page, and
    // looping forever against a server that ignores `offset`.
    auto *table = qobject_cast<QTableWidget *>(tabManager->currentWidget());
    if (table == nullptr || table->rowCount() == 0) {
        return;
    }

    // ScrollPerItem (the default) counts rows, not pixels: `value` is the top visible row
    // and `maximum` the last one that can be scrolled to. Prefetch half a page ahead so
    // scrolling never visibly stalls, without the fixed 300-row threshold firing the next
    // page the moment a page loads.
    const QScrollBar *bar = table->verticalScrollBar();
    const int prefetchRows = qMax(1, pageSize / 2);
    if (bar->maximum() - bar->value() > prefetchRows) {
        return;
    }

    fetchPage(currentOffset);
}

void TabLog::clearClicked()
{
    findUsername->clear();
    findIPAddress->clear();
    findGameName->clear();
    findGameID->clear();
    findMessage->clear();
    pastXDays->clear();
    maximumResults->clear();
    mainRoom->setChecked(false);
    gameRoom->setChecked(false);
    privateChat->setChecked(false);
    pastDays->setAutoExclusive(false);
    pastDays->setChecked(false);
    today->setAutoExclusive(false);
    today->setChecked(false);
    lastHour->setAutoExclusive(false);
    lastHour->setChecked(false);
    pastDays->setAutoExclusive(true);
    today->setAutoExclusive(true);
    lastHour->setAutoExclusive(true);
    currentOffset = 0;
    hasMoreResults = false;
    hasCurrentQuery = false;
    snapshotBefore.clear();
}

void TabLog::createDock()
{
    labelFindUserName = new QLabel(tr("Username: "));
    findUsername = new LineEditUnfocusable("");
    findUsername->setMaxLength(MAX_NAME_LENGTH);
    findUsername->setAlignment(Qt::AlignCenter);
    labelFindIPAddress = new QLabel(tr("IP Address: "));
    findIPAddress = new LineEditUnfocusable("");
    findIPAddress->setMaxLength(MAX_NAME_LENGTH);
    findIPAddress->setAlignment(Qt::AlignCenter);
    labelFindGameName = new QLabel(tr("Game Name: "));
    findGameName = new LineEditUnfocusable("");
    findGameName->setMaxLength(MAX_NAME_LENGTH);
    findGameName->setAlignment(Qt::AlignCenter);
    labelFindGameID = new QLabel(tr("GameID: "));
    findGameID = new LineEditUnfocusable("");
    findGameID->setMaxLength(MAX_NAME_LENGTH);
    findGameID->setAlignment(Qt::AlignCenter);
    labelMessage = new QLabel(tr("Message: "));
    findMessage = new LineEditUnfocusable("");
    findMessage->setMaxLength(MAX_TEXT_LENGTH);
    findMessage->setAlignment(Qt::AlignCenter);

    mainRoom = new QCheckBox(tr("Main Room"));
    gameRoom = new QCheckBox(tr("Game Room"));
    privateChat = new QCheckBox(tr("Private Chat"));
    if (canUseDeveloperCommands) {
        // Developers cannot query private conversations.
        privateChat->setVisible(false);
        // The developer family ignores the IP filter server-side, so showing
        // the field would silently unfilter the result by it. Hide it.
        labelFindIPAddress->setVisible(false);
        findIPAddress->setVisible(false);
    }

    pastDays = new QRadioButton(tr("Past X Days: "));
    today = new QRadioButton(tr("Today"));
    lastHour = new QRadioButton(tr("Last Hour"));
    pastXDays = new QSpinBox;
    pastXDays->setMaximum(20);

    labelMaximum = new QLabel(tr("Results per page: "));
    maximumResults = new QSpinBox;
    maximumResults->setMaximum(1000);
    maximumResults->setToolTip(tr("Page size for smooth scrolling: more rows load automatically as you scroll."));

    labelDescription = new QLabel(tr("Leave filters empty to browse recent room and game messages.\n"
                                     "More rows load automatically as you scroll."));

    getButton = new QPushButton(tr("Get Logs"));
    getButton->setAutoDefault(true);
    connect(getButton, &QPushButton::clicked, this, &TabLog::getClicked);

    clearButton = new QPushButton(tr("Clear Filters"));
    clearButton->setAutoDefault(true);
    connect(clearButton, &QPushButton::clicked, this, &TabLog::clearClicked);

    criteriaGrid = new QGridLayout;
    criteriaGrid->addWidget(labelFindUserName, 0, 0);
    criteriaGrid->addWidget(findUsername, 0, 1);
    criteriaGrid->addWidget(labelFindIPAddress, 1, 0);
    criteriaGrid->addWidget(findIPAddress, 1, 1);
    criteriaGrid->addWidget(labelFindGameName, 2, 0);
    criteriaGrid->addWidget(findGameName, 2, 1);
    criteriaGrid->addWidget(labelFindGameID, 3, 0);
    criteriaGrid->addWidget(findGameID, 3, 1);
    criteriaGrid->addWidget(labelMessage, 4, 0);
    criteriaGrid->addWidget(findMessage, 4, 1);

    criteriaGroupBox = new QGroupBox(tr("Filters"));
    criteriaGroupBox->setLayout(criteriaGrid);
    criteriaGroupBox->setMaximumSize(500, 300);
    criteriaGroupBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    locationGrid = new QGridLayout;
    locationGrid->addWidget(mainRoom, 0, 0);
    locationGrid->addWidget(gameRoom, 0, 1);
    locationGrid->addWidget(privateChat, 0, 2);

    locationGroupBox = new QGroupBox(tr("Log Locations"));
    locationGroupBox->setLayout(locationGrid);

    rangeGrid = new QGridLayout;
    rangeGrid->addWidget(pastDays, 0, 0);
    rangeGrid->addWidget(pastXDays, 0, 1);
    rangeGrid->addWidget(today, 0, 2);
    rangeGrid->addWidget(lastHour, 0, 3);

    rangeGroupBox = new QGroupBox(tr("Date Range"));
    rangeGroupBox->setLayout(rangeGrid);

    maxResultsGrid = new QGridLayout;
    maxResultsGrid->addWidget(labelMaximum, 0, 0);
    maxResultsGrid->addWidget(maximumResults, 0, 1);

    maxResultsGroupBox = new QGroupBox(tr("Results per page"));
    maxResultsGroupBox->setLayout(maxResultsGrid);

    descriptionGrid = new QGridLayout;
    descriptionGrid->addWidget(labelDescription, 0, 0);

    descriptionGroupBox = new QGroupBox(tr(""));
    descriptionGroupBox->setLayout(descriptionGrid);

    buttonGrid = new QGridLayout;
    buttonGrid->addWidget(getButton, 0, 0);
    buttonGrid->addWidget(clearButton, 0, 1);

    buttonGroupBox = new QGroupBox(tr(""));
    buttonGroupBox->setLayout(buttonGrid);

    mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(criteriaGroupBox);
    mainLayout->addWidget(locationGroupBox);
    mainLayout->addWidget(rangeGroupBox);
    mainLayout->addWidget(maxResultsGroupBox);
    mainLayout->addWidget(descriptionGroupBox);
    mainLayout->addWidget(buttonGroupBox);
    mainLayout->setAlignment(Qt::AlignCenter);

    searchDockContents = new QWidget(this);
    searchDockContents->setLayout(mainLayout);

    searchDock = new QDockWidget(this);
    searchDock->setFeatures(QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable);
    searchDock->setWidget(searchDockContents);
}

void TabLog::appendChatMessage(QTableWidget *table,
                               int &counter,
                               const QString &time,
                               const QString &senderName,
                               const QString &senderIp,
                               const QString &messageText,
                               const QString &targetId,
                               const QString &targetName,
                               const QString &channel)
{
    table->insertRow(counter);
    table->setItem(counter, 0, new QTableWidgetItem(time));
    table->setItem(counter, 1, new QTableWidgetItem(senderName));
    table->setItem(counter, 2, new QTableWidgetItem(senderIp));
    table->setItem(counter, 3, new QTableWidgetItem(messageText));
    table->setItem(counter, 4, new QTableWidgetItem(targetId));
    table->setItem(counter, 5, new QTableWidgetItem(targetName));
    if (table->columnCount() > 6) {
        table->setItem(counter, 6, new QTableWidgetItem(channel));
    }
    ++counter;
}

void TabLog::viewLogHistory_processResponse(const Response &resp)
{
    const Response_ViewLogHistory &response = resp.GetExtension(Response_ViewLogHistory::ext);
    const bool isFirstPage = (currentOffset == 0);
    isFetching = false;
    getButton->setEnabled(true);

    if (resp.response_code() != Response::RespOk) {
        QMessageBox::critical(static_cast<QWidget *>(parent()), tr("Message History"),
                              tr("Failed to collect message history information."));
        return;
    }

    if (isFirstPage && response.log_message_size() == 0) {
        QMessageBox::information(static_cast<QWidget *>(parent()), tr("Message History"),
                                 tr("There are no messages for the selected filters."));
        hasMoreResults = false;
        return;
    }

    const QString thisPageStart = response.log_message_size() > 0 ? pageStartKey(response) : QString();

    if (isFirstPage) {
        // Rows come newest first, so the first one fixes the snapshot every later page is
        // bounded by. Without it, each message logged between two page fetches pushes the
        // window down by one and repeats a row the moderator already has.
        snapshotBefore = QString::fromStdString(response.log_message(0).time());
    } else if (!response.has_has_more() && !thisPageStart.isEmpty() && thisPageStart == lastPageStartKey) {
        // A server that predates `offset` ignores it and returns the same page again. It
        // also predates `has_more`, so the full-page heuristic below can never prove
        // exhaustion and every scroll would append the same rows forever. The identical
        // first row is the proof; stop instead of looping.
        hasMoreResults = false;
        return;
    }
    lastPageStartKey = thisPageStart;

    for (int i = 0; i < response.log_message_size(); ++i) {
        ServerInfo_ChatMessage message = response.log_message(i);
        const QString targetType = QString::fromStdString(message.target_type());
        if (targetType == "room" || targetType == "room_channel") {
            appendChatMessage(roomTable, roomCounter, QString::fromStdString(message.time()),
                              QString::fromStdString(message.sender_name()),
                              QString::fromStdString(message.sender_ip()), QString::fromStdString(message.message()),
                              QString::fromStdString(message.target_id()),
                              QString::fromStdString(message.target_name()), QString::fromStdString(message.channel()));
        }

        if (targetType == "game") {
            appendChatMessage(gameTable, gameCounter, QString::fromStdString(message.time()),
                              QString::fromStdString(message.sender_name()),
                              QString::fromStdString(message.sender_ip()), QString::fromStdString(message.message()),
                              QString::fromStdString(message.target_id()),
                              QString::fromStdString(message.target_name()));
        }

        if (targetType == "chat") {
            appendChatMessage(chatTable, chatCounter, QString::fromStdString(message.time()),
                              QString::fromStdString(message.sender_name()),
                              QString::fromStdString(message.sender_ip()), QString::fromStdString(message.message()),
                              QString::fromStdString(message.target_id()),
                              QString::fromStdString(message.target_name()));
        }
    }

    currentOffset += response.log_message_size();
    if (response.has_has_more()) {
        hasMoreResults = response.has_more();
    } else {
        // Legacy server without has_more: a full page implies more may exist.
        hasMoreResults = (response.log_message_size() >= pageSize);
    }

    if (roomCounter) {
        roomTable->show();
        if (isFirstPage) {
            roomTable->resizeColumnsToContents();
        }
    } else {
        roomTable->hide();
    }

    if (gameCounter) {
        if (isFirstPage) {
            gameTable->resizeColumnsToContents();
        }
        gameTable->show();
    } else {
        gameTable->hide();
    }

    if (chatCounter) {
        if (isFirstPage) {
            chatTable->resizeColumnsToContents();
        }
        chatTable->show();
    } else {
        chatTable->hide();
    }

    // The new rows may already fill less than a viewport (or the user is
    // still parked at the bottom), so chain the next page without waiting
    // for another scroll event. The prefetch stays invisible: rows simply
    // keep appearing as you scroll.
    checkFetchMore();
}

void TabLog::restartLayout()
{
    searchDock->setFloating(false);
    addDockWidget(Qt::LeftDockWidgetArea, searchDock);
    searchDock->setVisible(true);
}
