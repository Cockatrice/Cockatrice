#include "tab_report.h"

#include "../utility/report_utils.h"
#include "abstract_client.h"
#include "tab_supervisor.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDateTime>
#include <QFontDatabase>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSplitter>
#include <QTableWidget>
#include <QTextEdit>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>
#include <libcockatrice/protocol/pb/command_replay_download_by_game_id.pb.h>
#include <libcockatrice/protocol/pb/command_report_add_comment.pb.h>
#include <libcockatrice/protocol/pb/command_report_assign.pb.h>
#include <libcockatrice/protocol/pb/command_report_details.pb.h>
#include <libcockatrice/protocol/pb/command_report_list.pb.h>
#include <libcockatrice/protocol/pb/command_report_resolve.pb.h>
#include <libcockatrice/protocol/pb/command_report_stats.pb.h>
#include <libcockatrice/protocol/pb/command_report_user_info.pb.h>
#include <libcockatrice/protocol/pb/game_replay.pb.h>
#include <libcockatrice/protocol/pb/response_replay_download_by_game_id.pb.h>
#include <libcockatrice/protocol/pb/response_report_details.pb.h>
#include <libcockatrice/protocol/pb/response_report_list.pb.h>
#include <libcockatrice/protocol/pb/response_report_stats.pb.h>
#include <libcockatrice/protocol/pb/response_report_user_info.pb.h>
#include <libcockatrice/protocol/pb/serverinfo_report.pb.h>
#include <libcockatrice/protocol/pending_command.h>

namespace
{
constexpr int COL_ID = 0;
constexpr int COL_TIME = 1;
constexpr int COL_REPORTER = 2;
constexpr int COL_REPORTED = 3;
constexpr int COL_CATEGORY = 4;
constexpr int COL_GAMEID = 5;
constexpr int COL_STATUS = 6;
constexpr int COL_ASSIGNED = 7;
constexpr int COL_REPLAY = 8;
constexpr int COL_ROOM = 9;
constexpr int COL_COUNT = 10;
constexpr int REFRESH_INTERVAL_MS = 300000;
} // namespace

TabReport::TabReport(TabSupervisor *_tabSupervisor, AbstractClient *_client) : Tab(_tabSupervisor), client(_client)
{
    auto *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    searchEdit = new QLineEdit;
    searchEdit->setClearButtonEnabled(true);
    connect(searchEdit, &QLineEdit::textChanged, this, &TabReport::applyFilters);

    statusFilter = new QComboBox;
    connect(statusFilter, &QComboBox::currentIndexChanged, this, &TabReport::applyFilters);

    unresolvedOnlyBox = new QCheckBox;
    unresolvedOnlyBox->setChecked(true);
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
    connect(unresolvedOnlyBox, &QCheckBox::checkStateChanged, this, &TabReport::refreshList);
#else
    connect(unresolvedOnlyBox, &QCheckBox::stateChanged, this, &TabReport::refreshList);
#endif

    refreshButton = new QPushButton;
    connect(refreshButton, &QPushButton::clicked, this, &TabReport::refreshList);

    refreshTimer = new QTimer(this);
    refreshTimer->setInterval(REFRESH_INTERVAL_MS);
    connect(refreshTimer, &QTimer::timeout, this, [this]() {
        if (tabSupervisor->currentWidget() == this) {
            refreshList();
        }
    });
    refreshTimer->start();

    auto *topBar = new QHBoxLayout;
    topBar->addWidget(searchEdit);
    topBar->addWidget(statusFilter);
    topBar->addWidget(unresolvedOnlyBox);
    topBar->addStretch();
    topBar->addWidget(refreshButton);

    statsLabel = new QLabel;

    table = new QTableWidget(0, COL_COUNT);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSortingEnabled(true);
    table->verticalHeader()->setVisible(false);
    table->setAlternatingRowColors(true);
    table->horizontalHeader()->setSectionResizeMode(COL_TIME, QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(COL_REPORTED, QHeaderView::Stretch);
    table->horizontalHeader()->setSectionResizeMode(COL_REPORTER, QHeaderView::ResizeToContents);
    connect(table, &QTableWidget::itemSelectionChanged, this, &TabReport::onSelectionChanged);

    descGroup = new QGroupBox;
    descriptionEdit = new QTextEdit;
    descriptionEdit->setReadOnly(true);
    auto *descLayout = new QVBoxLayout(descGroup);
    descLayout->setContentsMargins(4, 4, 4, 4);
    descLayout->addWidget(descriptionEdit);

    chatGroup = new QGroupBox;
    chatLogEdit = new QTextEdit;
    chatLogEdit->setReadOnly(true);
    QFont monoFont("monospace");
    monoFont.setStyleHint(QFont::Monospace);
    const int systemPointSize = QFontDatabase::systemFont(QFontDatabase::GeneralFont).pointSize();
    if (systemPointSize > 0) {
        monoFont.setPointSize(systemPointSize);
    }
    chatLogEdit->setFont(monoFont);
    auto *chatLayout = new QVBoxLayout(chatGroup);
    chatLayout->setContentsMargins(4, 4, 4, 4);
    chatLayout->addWidget(chatLogEdit);

    commentsGroup = new QGroupBox;
    commentsEdit = new QTextEdit;
    commentsEdit->setReadOnly(true);
    auto *commentInputRow = new QHBoxLayout;
    commentInput = new QLineEdit;
    commentButton = new QPushButton;
    commentButton->setEnabled(false);
    connect(commentButton, &QPushButton::clicked, this, &TabReport::addComment);
    connect(commentInput, &QLineEdit::returnPressed, this, &TabReport::addComment);
    commentInputRow->addWidget(commentInput);
    commentInputRow->addWidget(commentButton);
    auto *commentsLayout = new QVBoxLayout(commentsGroup);
    commentsLayout->setContentsMargins(4, 4, 4, 4);
    commentsLayout->addWidget(commentsEdit);
    commentsLayout->addLayout(commentInputRow);

    userContextGroup = new QGroupBox;
    userContextName = new QLabel;
    userContextAccountAge = new QLabel;
    userContextReports = new QLabel;
    userContextBans = new QLabel;
    userContextWarns = new QLabel;
    userContextNotes = new QTextEdit;
    userContextNotes->setReadOnly(true);
    userContextNotes->setMaximumHeight(80);
    userContextRecentReports = new QTextEdit;
    userContextRecentReports->setReadOnly(true);
    userContextRecentReports->setMaximumHeight(120);
    userContextUserLabel = new QLabel;
    userContextAgeLabel = new QLabel;
    userContextReportsLabel = new QLabel;
    userContextBansLabel = new QLabel;
    userContextWarnsLabel = new QLabel;
    userContextNotesLabel = new QLabel;
    userContextRecentReportsLabel = new QLabel;
    auto *ucLayout = new QGridLayout(userContextGroup);
    ucLayout->setContentsMargins(8, 8, 8, 8);
    ucLayout->addWidget(userContextUserLabel, 0, 0);
    ucLayout->addWidget(userContextName, 0, 1, 1, 3);
    ucLayout->addWidget(userContextAgeLabel, 1, 0);
    ucLayout->addWidget(userContextAccountAge, 1, 1);
    ucLayout->addWidget(userContextReportsLabel, 1, 2);
    ucLayout->addWidget(userContextReports, 1, 3);
    ucLayout->addWidget(userContextBansLabel, 2, 0);
    ucLayout->addWidget(userContextBans, 2, 1);
    ucLayout->addWidget(userContextWarnsLabel, 2, 2);
    ucLayout->addWidget(userContextWarns, 2, 3);
    ucLayout->addWidget(userContextNotesLabel, 3, 0, Qt::AlignTop);
    ucLayout->addWidget(userContextNotes, 3, 1, 1, 3);
    ucLayout->addWidget(userContextRecentReportsLabel, 4, 0, 1, 4);
    ucLayout->addWidget(userContextRecentReports, 5, 0, 1, 4);
    userContextGroup->setVisible(false);

    statsGroup = new QGroupBox;
    statsGroup->setCheckable(true);
    statsTotalLabel = new QLabel;
    statsTrendLabel = new QLabel;
    statsCategoriesLabel = new QLabel;
    statsDetailText = new QTextEdit;
    statsDetailText->setReadOnly(true);
    statsDetailText->setMaximumHeight(150);
    auto *statsContent = new QWidget;
    auto *sgLayout = new QVBoxLayout(statsContent);
    sgLayout->setContentsMargins(8, 8, 8, 8);
    sgLayout->addWidget(statsTotalLabel);
    sgLayout->addWidget(statsTrendLabel);
    sgLayout->addWidget(statsCategoriesLabel);
    sgLayout->addWidget(statsDetailText);
    auto *statsGroupLayout = new QVBoxLayout(statsGroup);
    statsGroupLayout->setContentsMargins(0, 0, 0, 0);
    statsGroupLayout->addWidget(statsContent);
    statsGroup->setChecked(true);
    connect(statsGroup, &QGroupBox::toggled, this, [this, statsContent](bool checked) {
        statsContent->setVisible(checked);
        if (checked) {
            requestStats();
        }
    });

    detailSplitter = new QSplitter(Qt::Vertical);
    detailSplitter->addWidget(descGroup);
    detailSplitter->addWidget(chatGroup);
    detailSplitter->addWidget(commentsGroup);
    detailSplitter->setStretchFactor(0, 1);
    detailSplitter->setStretchFactor(1, 1);
    detailSplitter->setStretchFactor(2, 2);

    assignButton = new QPushButton;
    resolveButton = new QPushButton;
    resolveWithNoteButton = new QPushButton;
    dismissButton = new QPushButton;
    viewReplayButton = new QPushButton;
    joinGameButton = new QPushButton;
    connect(assignButton, &QPushButton::clicked, this, &TabReport::assignReport);
    connect(resolveButton, &QPushButton::clicked, this, [this]() { resolveReport(false, false); });
    connect(resolveWithNoteButton, &QPushButton::clicked, this, [this]() { resolveReport(false, true); });
    connect(dismissButton, &QPushButton::clicked, this, [this]() { resolveReport(true, true); });
    connect(viewReplayButton, &QPushButton::clicked, this, &TabReport::viewReplay);
    connect(joinGameButton, &QPushButton::clicked, this, &TabReport::joinGame);

    statusLabel = new QLabel;

    auto *actionBar = new QHBoxLayout;
    actionBar->addWidget(assignButton);
    actionBar->addWidget(resolveButton);
    actionBar->addWidget(resolveWithNoteButton);
    actionBar->addWidget(dismissButton);
    actionBar->addSpacing(20);
    actionBar->addWidget(viewReplayButton);
    actionBar->addWidget(joinGameButton);
    actionBar->addStretch();
    actionBar->addWidget(statusLabel);

    auto *layout = new QVBoxLayout(centralWidget);
    layout->addLayout(topBar);
    layout->addWidget(statsLabel);
    layout->addWidget(table, 2);
    layout->addWidget(detailSplitter, 1);
    layout->addWidget(userContextGroup);
    layout->addWidget(statsGroup);
    layout->addLayout(actionBar);

    retranslateUi();

    setActionsEnabled(false);
    refreshList();
}

void TabReport::retranslateUi()
{
    searchEdit->setPlaceholderText(tr("Search by username, category..."));
    statusFilter->clear();
    statusFilter->addItem(tr("All Statuses"), "");
    statusFilter->addItem(tr("Open"), "open");
    statusFilter->addItem(tr("Assigned"), "assigned");
    statusFilter->addItem(tr("Resolved"), "resolved");
    statusFilter->addItem(tr("Dismissed"), "dismissed");
    unresolvedOnlyBox->setText(tr("Unresolved only"));
    refreshButton->setText(tr("Refresh"));

    table->setHorizontalHeaderLabels({tr("#"), tr("Time"), tr("Reporter"), tr("Reported User"), tr("Category"),
                                      tr("Game ID"), tr("Status"), tr("Assigned To"), tr("Replay"), tr("Room")});

    descGroup->setTitle(tr("Description"));
    chatGroup->setTitle(tr("Chat Log Context"));
    chatGroup->setToolTip(
        tr("Chat log attached by the reporter. It is captured from their client and may be incomplete or edited."));
    commentsGroup->setTitle(tr("Comments / Thread"));
    descriptionEdit->setPlaceholderText(tr("No description."));
    chatLogEdit->setPlaceholderText(tr("No chat log attached."));
    commentsEdit->setPlaceholderText(tr("No comments yet."));
    commentInput->setPlaceholderText(tr("Type a reply..."));
    commentButton->setText(tr("Send"));

    userContextGroup->setTitle(tr("Reported User Context"));
    userContextUserLabel->setText(tr("User:"));
    userContextAgeLabel->setText(tr("Account Age:"));
    userContextReportsLabel->setText(tr("Reports:"));
    userContextBansLabel->setText(tr("Bans:"));
    userContextWarnsLabel->setText(tr("Warns:"));
    userContextNotesLabel->setText(tr("Admin Notes:"));
    userContextRecentReportsLabel->setText(tr("Recent Reports Against User:"));
    userContextAccountAge->setText(QString());
    userContextReports->setText(QString());
    userContextBans->setText(QString());
    userContextWarns->setText(QString());

    statsGroup->setTitle(tr("Report Statistics"));

    assignButton->setText(tr("Assign to Me"));
    resolveButton->setText(tr("Resolve"));
    resolveWithNoteButton->setText(tr("Resolve with note..."));
    dismissButton->setText(tr("Dismiss..."));
    viewReplayButton->setText(tr("View Replay"));
    joinGameButton->setText(tr("Join Game"));
}

void TabReport::refreshList()
{
    selectedReportIdBeforeRefresh = selectedReportId();
    commentDraftBeforeRefresh = commentInput->text();
    previousSelectedReportValid = selectedReportInfo(previousSelectedReport);

    refreshButton->setEnabled(false);
    statusLabel->setText(tr("Loading..."));
    table->setRowCount(0);
    allReports.clear();
    filteredReports.clear();
    commentButton->setEnabled(false);
    setActionsEnabled(false);

    Command_ReportList cmd;
    cmd.set_unresolved_only(unresolvedOnlyBox->isChecked());

    PendingCommand *pend = client->prepareModeratorCommand(cmd);
    connect(pend, &PendingCommand::finished, this, &TabReport::reportListResponse);
    client->sendCommand(pend);
}

void TabReport::reportListResponse(const Response &response)
{
    refreshButton->setEnabled(true);

    if (response.response_code() != Response::RespOk) {
        statusLabel->setText(tr("Failed to load reports."));
        return;
    }

    const Response_ReportList &resp = response.GetExtension(Response_ReportList::ext);
    allReports.clear();
    for (int i = 0; i < resp.reports_size(); ++i) {
        allReports.append(resp.reports(i));
    }

    applyFilters();
    updateStats();

    if (selectedReportIdBeforeRefresh >= 0) {
        bool found = false;
        for (int row = 0; row < table->rowCount(); ++row) {
            if (table->item(row, COL_ID) &&
                table->item(row, COL_ID)->data(Qt::UserRole).toInt() == selectedReportIdBeforeRefresh) {
                found = true;
                {
                    QSignalBlocker blocker(table);
                    table->setCurrentCell(row, 0);
                }
                break;
            }
        }

        if (found) {
            bool dataChanged = false;
            ServerInfo_Report newReport;
            if (!selectedReportInfo(newReport) || !previousSelectedReportValid ||
                previousSelectedReport.description() != newReport.description() ||
                previousSelectedReport.status() != newReport.status() ||
                previousSelectedReport.resolution_note() != newReport.resolution_note() ||
                previousSelectedReport.assigned_mod_name() != newReport.assigned_mod_name()) {
                dataChanged = true;
            }

            if (dataChanged || detailsRequestedReportId != selectedReportIdBeforeRefresh) {
                loadReportDetails(selectedReportIdBeforeRefresh);
            }
            updateActionStates();
        } else {
            descriptionEdit->clear();
            chatLogEdit->clear();
            commentsEdit->clear();
            setActionsEnabled(false);
            userContextGroup->setVisible(false);
        }
    }

    if (commentInput->text().isEmpty()) {
        commentInput->setText(commentDraftBeforeRefresh);
    }

    if (statsGroup->isChecked()) {
        requestStats();
    }
}

void TabReport::applyFilters()
{
    filteredReports.clear();

    const QString searchText = searchEdit->text().toLower();
    const QString statusFilterValue = statusFilter->currentData().toString();

    for (const ServerInfo_Report &r : allReports) {
        bool matchesSearch = searchText.isEmpty() ||
                             QString::fromStdString(r.reporter_name()).toLower().contains(searchText) ||
                             QString::fromStdString(r.reported_user_name()).toLower().contains(searchText) ||
                             QString::fromStdString(r.category()).toLower().contains(searchText) ||
                             QString::fromStdString(r.description()).toLower().contains(searchText);

        bool matchesStatus = statusFilterValue.isEmpty() || QString::fromStdString(r.status()) == statusFilterValue;

        if (matchesSearch && matchesStatus) {
            filteredReports.append(r);
        }
    }

    table->setSortingEnabled(false);
    table->setRowCount(filteredReports.size());

    for (int row = 0; row < filteredReports.size(); ++row) {
        const ServerInfo_Report &r = filteredReports[row];

        report_utils::fillReportTableRow(table, row, r, COL_ID, COL_TIME, COL_REPORTED, COL_CATEGORY, COL_GAMEID,
                                         COL_STATUS, COL_ASSIGNED);

        table->setItem(row, COL_REPORTER, new QTableWidgetItem(QString::fromStdString(r.reporter_name())));

        table->setItem(row, COL_REPLAY,
                       new QTableWidgetItem(r.has_replay_id() && r.replay_id() > 0 ? tr("Yes") : tr("No")));

        table->setItem(row, COL_ROOM,
                       new QTableWidgetItem(r.has_room_id() && r.room_id() > 0 ? QString::number(r.room_id()) : ""));
    }

    table->setSortingEnabled(true);
    table->resizeColumnsToContents();
    table->horizontalHeader()->setSectionResizeMode(COL_REPORTED, QHeaderView::Stretch);

    statusLabel->setText(tr("%1 report(s)").arg(filteredReports.size()));
}

void TabReport::updateStats()
{
    int open = 0, assigned = 0, resolved = 0, dismissed = 0;
    for (const ServerInfo_Report &r : allReports) {
        QString status = QString::fromStdString(r.status());
        if (status == "open") {
            open++;
        } else if (status == "assigned") {
            assigned++;
        } else if (status == "resolved") {
            resolved++;
        } else if (status == "dismissed") {
            dismissed++;
        }
    }

    statsLabel->setText(tr("%1 open  |  %2 assigned  |  %3 resolved  |  %4 dismissed")
                            .arg(open)
                            .arg(assigned)
                            .arg(resolved)
                            .arg(dismissed));
}

void TabReport::onSelectionChanged()
{
    const int row = table->currentRow();
    if (row < 0 || !table->item(row, COL_ID)) {
        descriptionEdit->clear();
        chatLogEdit->clear();
        commentsEdit->clear();
        commentInput->clear();
        commentButton->setEnabled(false);
        setActionsEnabled(false);
        userContextGroup->setVisible(false);
        return;
    }

    const int reportId = table->item(row, COL_ID)->data(Qt::UserRole).toInt();
    loadReportDetails(reportId);
    updateActionStates();
}

void TabReport::loadReportDetails(int reportId)
{
    detailsRequestedReportId = reportId;

    for (const ServerInfo_Report &r : filteredReports) {
        if (r.report_id() == reportId) {
            descriptionEdit->setPlainText(QString::fromStdString(r.description()));

            QString reportedUser = QString::fromStdString(r.reported_user_name());
            if (!reportedUser.isEmpty()) {
                requestUserInfo(reportedUser);
            } else {
                userContextGroup->setVisible(false);
            }

            QString status = QString::fromStdString(r.status());
            bool canComment = (status == "open" || status == "assigned");
            commentButton->setEnabled(canComment);
            commentInput->setEnabled(canComment);
            if (!canComment) {
                commentInput->setPlaceholderText(tr("This report is closed."));
            } else {
                commentInput->setPlaceholderText(tr("Type a reply..."));
            }
            break;
        }
    }

    chatLogEdit->setPlainText(tr("Loading..."));
    commentsEdit->setPlainText(tr("Loading..."));

    Command_ReportDetails cmd;
    cmd.set_report_id(reportId);

    const int seq = ++detailsRequestSeq;
    PendingCommand *pend = client->prepareSessionCommand(cmd);
    connect(pend, &PendingCommand::finished, this, [this, seq](const Response &response) {
        if (seq != detailsRequestSeq) {
            return;
        }
        reportDetailsResponse(response);
    });
    client->sendCommand(pend);
}

void TabReport::reportDetailsResponse(const Response &response)
{
    if (response.response_code() != Response::RespOk) {
        if (selectedReportId() == detailsRequestedReportId) {
            chatLogEdit->clear();
            commentsEdit->setPlainText(tr("Failed to load report details."));
        }
        return;
    }

    const Response_ReportDetails &resp = response.GetExtension(Response_ReportDetails::ext);
    const ServerInfo_Report &r = resp.report();

    if (selectedReportId() != r.report_id()) {
        return;
    }

    report_utils::renderReportDetails(chatLogEdit, commentsEdit, r, tr("No comments yet."), tr("[Moderator]"),
                                      tr("[Reporter]"));
}

void TabReport::updateActionStates()
{
    const int row = table->currentRow();
    if (row < 0 || !table->item(row, COL_STATUS)) {
        setActionsEnabled(false);
        return;
    }

    ServerInfo_Report report;
    if (!selectedReportInfo(report)) {
        setActionsEnabled(false);
        return;
    }

    const QString status = QString::fromStdString(report.status());
    assignButton->setEnabled(status == "open");
    resolveButton->setEnabled(status == "open" || status == "assigned");
    resolveWithNoteButton->setEnabled(status == "open" || status == "assigned");
    dismissButton->setEnabled(status == "open" || status == "assigned");

    const bool hasGameId = report.game_id() > 0;
    const bool hasReplay = hasGameId && report.has_replay_id() && report.replay_id() > 0;
    viewReplayButton->setEnabled(hasReplay);
    joinGameButton->setEnabled(hasGameId && report.has_room_id() && report.room_id() > 0);
}

void TabReport::setActionsEnabled(bool enabled)
{
    assignButton->setEnabled(enabled);
    resolveButton->setEnabled(enabled);
    resolveWithNoteButton->setEnabled(enabled);
    dismissButton->setEnabled(enabled);
    viewReplayButton->setEnabled(false);
    joinGameButton->setEnabled(false);
    commentButton->setEnabled(enabled);
    commentInput->setEnabled(enabled);
}

int TabReport::selectedReportId() const
{
    const int row = table->currentRow();
    if (row < 0 || !table->item(row, COL_ID)) {
        return -1;
    }
    return table->item(row, COL_ID)->data(Qt::UserRole).toInt();
}

bool TabReport::selectedReportInfo(ServerInfo_Report &info) const
{
    const int reportId = selectedReportId();
    if (reportId < 0) {
        return false;
    }

    for (const ServerInfo_Report &r : filteredReports) {
        if (r.report_id() == reportId) {
            info = r;
            return true;
        }
    }

    return false;
}

void TabReport::assignReport()
{
    const int reportId = selectedReportId();
    if (reportId < 0) {
        return;
    }

    setActionsEnabled(false);
    statusLabel->setText(tr("Assigning..."));

    Command_ReportAssign cmd;
    cmd.set_report_id(reportId);

    PendingCommand *pend = client->prepareModeratorCommand(cmd);
    connect(pend, &PendingCommand::finished, this, &TabReport::assignResponse);
    client->sendCommand(pend);
}

void TabReport::assignResponse(const Response &response)
{
    if (response.response_code() == Response::RespOk) {
        statusLabel->setText(tr("Assigned."));
        refreshList();
    } else {
        statusLabel->setText(tr("Assignment failed."));
        updateActionStates();
    }
}

void TabReport::resolveReport(bool dismissed, bool promptNote)
{
    const int reportId = selectedReportId();
    if (reportId < 0) {
        return;
    }

    QString note;
    if (promptNote) {
        bool ok;
        note = QInputDialog::getText(this, dismissed ? tr("Dismiss Report") : tr("Resolve Report"),
                                     dismissed ? tr("Optional note:") : tr("Resolution note (optional):"),
                                     QLineEdit::Normal, QString(), &ok);
        if (!ok) {
            return;
        }
    }

    setActionsEnabled(false);
    statusLabel->setText(dismissed ? tr("Dismissing...") : tr("Resolving..."));

    Command_ReportResolve cmd;
    cmd.set_report_id(reportId);
    cmd.set_dismissed(dismissed);
    if (!note.isEmpty()) {
        cmd.set_resolution_note(note.toStdString());
    }

    PendingCommand *pend = client->prepareModeratorCommand(cmd);
    connect(pend, &PendingCommand::finished, this, &TabReport::resolveResponse);
    client->sendCommand(pend);
}

void TabReport::resolveResponse(const Response &response)
{
    if (response.response_code() == Response::RespOk) {
        statusLabel->setText(tr("Done."));
        refreshList();
    } else {
        statusLabel->setText(tr("Action failed."));
        updateActionStates();
    }
}

void TabReport::viewReplay()
{
    ServerInfo_Report report;
    if (!selectedReportInfo(report) || report.game_id() <= 0) {
        return;
    }

    setActionsEnabled(false);
    statusLabel->setText(tr("Loading replay..."));

    Command_ReplayDownloadByGameId cmd;
    cmd.set_game_id(report.game_id());

    PendingCommand *pend = client->prepareModeratorCommand(cmd);
    connect(pend, &PendingCommand::finished, this, &TabReport::viewReplayResponse);
    client->sendCommand(pend);
}

void TabReport::viewReplayResponse(const Response &response)
{
    if (response.response_code() != Response::RespOk) {
        statusLabel->setText(tr("No replay available for this game."));
        updateActionStates();
        return;
    }

    const Response_ReplayDownloadByGameId &resp = response.GetExtension(Response_ReplayDownloadByGameId::ext);
    GameReplay *replay = new GameReplay;
    if (replay->ParseFromString(resp.replay_data())) {
        emit openReplay(replay);
        statusLabel->setText(tr("Replay opened."));
    } else {
        delete replay;
        statusLabel->setText(tr("Failed to parse replay."));
    }

    updateActionStates();
}

void TabReport::joinGame()
{
    ServerInfo_Report report;
    if (!selectedReportInfo(report) || report.game_id() <= 0) {
        return;
    }

    const int roomId = report.has_room_id() ? report.room_id() : -1;
    if (roomId <= 0) {
        statusLabel->setText(tr("No room recorded for this report, use the replay instead."));
        return;
    }

    emit requestJoinGame(report.game_id(), roomId);
    statusLabel->setText(tr("Attempting to join game..."));
}

void TabReport::addComment()
{
    const int reportId = selectedReportId();
    if (reportId < 0) {
        return;
    }

    QString text = commentInput->text().trimmed();
    if (text.isEmpty()) {
        return;
    }

    commentButton->setEnabled(false);

    Command_ReportAddComment cmd;
    cmd.set_report_id(reportId);
    cmd.set_comment(text.toStdString());

    PendingCommand *pend = client->prepareSessionCommand(cmd);
    connect(pend, &PendingCommand::finished, this, &TabReport::addCommentResponse);
    client->sendCommand(pend);
}

void TabReport::addCommentResponse(const Response &response)
{
    if (response.response_code() == Response::RespOk) {
        commentInput->clear();
        refreshList();
    } else {
        commentButton->setEnabled(true);
        statusLabel->setText(tr("Failed to send comment."));
    }
}

void TabReport::requestUserInfo(const QString &userName)
{
    if (userName.isEmpty()) {
        userContextGroup->setVisible(false);
        return;
    }

    lastRequestedUser = userName;
    userContextGroup->setVisible(true);
    userContextName->setText(userName);
    userContextAccountAge->setText(tr("Loading..."));
    userContextReports->clear();
    userContextBans->clear();
    userContextWarns->clear();
    userContextNotes->clear();
    userContextRecentReports->clear();

    Command_ReportUserInfo cmd;
    cmd.set_user_name(userName.toStdString());

    PendingCommand *pend = client->prepareModeratorCommand(cmd);
    connect(pend, &PendingCommand::finished, this, &TabReport::userInfoResponse);
    client->sendCommand(pend);
}

void TabReport::userInfoResponse(const Response &response)
{
    if (response.response_code() != Response::RespOk) {
        userContextAccountAge->setText(tr("Error loading user info."));
        return;
    }

    const Response_ReportUserInfo &resp = response.GetExtension(Response_ReportUserInfo::ext);

    if (resp.has_user_name() && QString::fromStdString(resp.user_name()) != lastRequestedUser) {
        return;
    }

    QDateTime regTime = QDateTime::fromSecsSinceEpoch(resp.registration_time());
    qint64 daysSinceReg = regTime.daysTo(QDateTime::currentDateTime());
    userContextAccountAge->setText(tr("%1 days (since %2)").arg(daysSinceReg).arg(regTime.toString("yyyy-MM-dd")));

    userContextReports->setText(QString::number(resp.total_reports()));
    userContextBans->setText(QString::number(resp.total_bans()));
    userContextWarns->setText(QString::number(resp.total_warns()));

    if (resp.has_admin_notes() && !resp.admin_notes().empty()) {
        userContextNotes->setPlainText(QString::fromStdString(resp.admin_notes()));
    } else {
        userContextNotes->setPlainText(tr("(none)"));
    }

    userContextRecentReports->clear();
    if (resp.recent_reports_size() == 0) {
        userContextRecentReports->setPlainText(tr("No previous reports against this user."));
    } else {
        for (int i = 0; i < resp.recent_reports_size(); ++i) {
            const ServerInfo_Report &r = resp.recent_reports(i);
            QDateTime dt = QDateTime::fromSecsSinceEpoch(r.report_time());
            userContextRecentReports->append(QString("[%1] #%2 by %3 [%4]: %5")
                                                 .arg(dt.toString("yyyy-MM-dd"))
                                                 .arg(r.report_id())
                                                 .arg(QString::fromStdString(r.reporter_name()))
                                                 .arg(QString::fromStdString(r.status()))
                                                 .arg(QString::fromStdString(r.category())));
        }
    }
}

void TabReport::requestStats()
{
    statsTotalLabel->setText(tr("Loading statistics..."));
    statsTrendLabel->clear();
    statsCategoriesLabel->clear();
    statsDetailText->clear();

    Command_ReportStats cmd;

    PendingCommand *pend = client->prepareModeratorCommand(cmd);
    connect(pend, &PendingCommand::finished, this, &TabReport::statsResponse);
    client->sendCommand(pend);
}

void TabReport::statsResponse(const Response &response)
{
    if (response.response_code() != Response::RespOk) {
        statsTotalLabel->setText(tr("Error loading statistics."));
        return;
    }

    const Response_ReportStats &resp = response.GetExtension(Response_ReportStats::ext);

    statsTotalLabel->setText(tr("Total: %1 reports (%2 open, %3 assigned, %4 resolved/dismissed)")
                                 .arg(resp.total_reports())
                                 .arg(resp.total_pending())
                                 .arg(resp.total_assigned())
                                 .arg(resp.total_resolved()));

    statsTrendLabel->setText(tr("Last 24h: %1  |  Last 7d: %2  |  Last 30d: %3  |  Avg resolution: %4h")
                                 .arg(resp.reports_last_24h())
                                 .arg(resp.reports_last_7d())
                                 .arg(resp.reports_last_30d())
                                 .arg(resp.avg_resolution_hours(), 0, 'f', 1));

    QString weekCompare =
        tr("This week: %1 vs last week: %2 (%3%)")
            .arg(resp.reports_this_week())
            .arg(resp.reports_last_week())
            .arg(resp.reports_last_week() > 0 ? QString::number(((resp.reports_this_week() - resp.reports_last_week()) *
                                                                 100.0 / resp.reports_last_week()),
                                                                'f', 0)
                 : resp.reports_this_week() > 0 ? "new"
                                                : "0");
    statsTrendLabel->setText(statsTrendLabel->text() + "  |  " + weekCompare);

    statsCategoriesLabel->setText(tr("By category:"));
    statsDetailText->clear();

    statsDetailText->append(tr("=== Top Categories ==="));
    for (int i = 0; i < resp.category_counts_size(); ++i) {
        const ReportCategoryCount &cc = resp.category_counts(i);
        QString cat = QString::fromStdString(cc.category());
        cat.replace('_', ' ');
        cat = cat.left(1).toUpper() + cat.mid(1);
        statsDetailText->append(QString("  %1: %2").arg(cat).arg(cc.count()));
    }

    statsDetailText->append(tr("\n=== Most Reported Users ==="));
    for (int i = 0; i < resp.top_reported_users_size(); ++i) {
        const ReportTopUser &tu = resp.top_reported_users(i);
        statsDetailText->append(
            QString("  %1: %2 reports").arg(QString::fromStdString(tu.user_name())).arg(tu.count()));
    }

    statsDetailText->append(tr("\n=== Top Reporters ==="));
    for (int i = 0; i < resp.top_reporters_size(); ++i) {
        const ReportTopUser &tu = resp.top_reporters(i);
        statsDetailText->append(
            QString("  %1: %2 reports filed").arg(QString::fromStdString(tu.user_name())).arg(tu.count()));
    }
}
