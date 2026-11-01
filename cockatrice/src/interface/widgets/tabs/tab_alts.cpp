#include "tab_alts.h"

#include "../utility/table_activation.h"
#include "abstract_client.h"
#include "libcockatrice/protocol/pb/response.pb.h"
#include "libcockatrice/protocol/pb/response_user_alts.pb.h"

#include <QAbstractItemView>
#include <QDateTime>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QLabel>
#include <QLineEdit>
#include <QList>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QSplitter>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QWidget>
#include <libcockatrice/protocol/pb/command_report_user_info.pb.h>
#include <libcockatrice/protocol/pb/moderator_commands.pb.h>
#include <libcockatrice/protocol/pb/response_ban_history.pb.h>
#include <libcockatrice/protocol/pb/response_report_user_info.pb.h>
#include <libcockatrice/protocol/pb/response_warn_history.pb.h>
#include <libcockatrice/protocol/pb/serverinfo_ban.pb.h>
#include <libcockatrice/protocol/pb/serverinfo_warning.pb.h>
#include <libcockatrice/protocol/pending_command.h>
#include <string>

namespace
{
constexpr int COL_ALTS_USER = 0;
constexpr int COL_ALTS_VIA = 1;
constexpr int COL_ALTS_DEPTH = 2;
constexpr int COL_ALTS_SHARED = 3;
constexpr int COL_ALTS_EMAIL = 4;
constexpr int COL_ALTS_CLIENTID = 5;
constexpr int COL_ALTS_REGISTERED = 6;
constexpr int COL_ALTS_LAST_LOGIN = 7;
constexpr int COL_ALTS_WARNS = 8;
constexpr int COL_ALTS_BANS = 9;
constexpr int COL_ALTS_ACTIVE = 10;
constexpr int COL_ALTS_COUNT = 11;

constexpr int COL_WARNS_TIME = 0;
constexpr int COL_WARNS_MODERATOR = 1;
constexpr int COL_WARNS_REASON = 2;
constexpr int COL_WARNS_COUNT = 3;

constexpr int COL_BANS_TIME = 0;
constexpr int COL_BANS_MODERATOR = 1;
constexpr int COL_BANS_LENGTH = 2;
constexpr int COL_BANS_REASON = 3;
constexpr int COL_BANS_COUNT = 4;

constexpr int MAX_ALT_DEPTH = 3;
// Starting default only (adjustable 1-100000): shared VPN exits routinely
// cover dozens of users, so 10 hides the obvious VPN noise but keeps
// small shared networks visible.
constexpr int DEFAULT_VPN_THRESHOLD = 10;

void configureAltTable(QTableWidget *table)
{
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->verticalHeader()->setVisible(false);
    table->setAlternatingRowColors(true);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    table->horizontalHeader()->setStretchLastSection(true);
}
} // namespace

TabAlts::TabAlts(TabSupervisor *_tabSupervisor, AbstractClient *_client, const QString &initialUser)
    : Tab(_tabSupervisor), client(_client)
{
    auto *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    searchEdit = new QLineEdit;
    searchEdit->setClearButtonEnabled(true);
    connect(searchEdit, &QLineEdit::returnPressed, this, &TabAlts::investigateUser);

    depthSpin = new QSpinBox;
    depthSpin->setMinimum(1);
    depthSpin->setMaximum(MAX_ALT_DEPTH);
    depthSpin->setValue(1);

    investigateButton = new QPushButton;
    connect(investigateButton, &QPushButton::clicked, this, &TabAlts::investigateUser);

    backButton = new QPushButton;
    backButton->setEnabled(false);
    connect(backButton, &QPushButton::clicked, this, &TabAlts::goBack);

    statusLabel = new QLabel;

    auto *topBar = new QHBoxLayout;
    topBar->addWidget(backButton);
    topBar->addWidget(searchEdit);
    topBar->addWidget(depthSpin);
    topBar->addWidget(investigateButton);

    auto makeChip = [this](const QString &text) {
        auto *chip = new QPushButton(text);
        chip->setCheckable(true);
        chip->setChecked(true);
        connect(chip, &QPushButton::toggled, this, &TabAlts::onFilterChanged);
        return chip;
    };
    emailChip = makeChip(QString());
    clientIdChip = makeChip(QString());
    ipChip = makeChip(QString());
    vpnLabel = new QLabel;
    vpnSpin = new QSpinBox;
    vpnSpin->setMinimum(1);
    vpnSpin->setMaximum(100000);
    vpnSpin->setValue(DEFAULT_VPN_THRESHOLD);
    connect(vpnSpin, &QSpinBox::valueChanged, this, &TabAlts::onFilterChanged);

    auto *filterBar = new QHBoxLayout;
    filterBar->addWidget(emailChip);
    filterBar->addWidget(clientIdChip);
    filterBar->addWidget(ipChip);
    filterBar->addWidget(vpnLabel);
    filterBar->addWidget(vpnSpin);
    filterBar->addStretch();

    altsTable = new QTableWidget(0, COL_ALTS_COUNT);
    configureAltTable(altsTable);
    connectTableActivation(altsTable, this, [this](const QModelIndex &index) {
        if (!index.isValid()) {
            return;
        }
        auto *userItem = altsTable->item(index.row(), COL_ALTS_USER);
        if (userItem != nullptr && !userItem->text().isEmpty() && userItem->text() != currentUser) {
            investigate(userItem->text());
        }
    });
    connect(altsTable, &QTableWidget::itemSelectionChanged, this, &TabAlts::onAltSelected);

    detailNameValue = new QLabel;
    detailCountsValue = new QLabel;
    detailNotesEdit = new QTextEdit;
    detailNotesEdit->setReadOnly(true);
    // Compact cap so the warn/ban tables stay visible without scrolling the tab.
    detailNotesEdit->setMaximumHeight(60);
    warnsTable = new QTableWidget(0, COL_WARNS_COUNT);
    configureAltTable(warnsTable);
    bansTable = new QTableWidget(0, COL_BANS_COUNT);
    configureAltTable(bansTable);

    auto *detailGrid = new QGridLayout;
    detailNameLabel = new QLabel;
    detailCountsLabel = new QLabel;
    detailNotesLabel = new QLabel;
    detailGrid->addWidget(detailNameLabel, 0, 0);
    detailGrid->addWidget(detailNameValue, 0, 1);
    detailGrid->addWidget(detailCountsLabel, 0, 2);
    detailGrid->addWidget(detailCountsValue, 0, 3);
    detailGrid->addWidget(detailNotesLabel, 1, 0);
    detailGrid->addWidget(detailNotesEdit, 1, 1, 1, 3);

    detailsGroup = new QGroupBox;
    auto *detailsLayout = new QVBoxLayout(detailsGroup);
    detailsLayout->addLayout(detailGrid);
    detailsLayout->addWidget(warnsTable, 1);
    detailsLayout->addWidget(bansTable, 1);

    auto *contentSplitter = new QSplitter(Qt::Horizontal);
    contentSplitter->addWidget(altsTable);
    contentSplitter->addWidget(detailsGroup);
    contentSplitter->setStretchFactor(0, 3);
    contentSplitter->setStretchFactor(1, 2);
    detailsGroup->setMinimumWidth(260);

    auto *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->addLayout(topBar);
    mainLayout->addLayout(filterBar);
    mainLayout->addWidget(statusLabel);
    mainLayout->addWidget(contentSplitter, 1);

    retranslateUi();
    clearDetails();
    investigate(initialUser);
}

void TabAlts::retranslateUi()
{
    searchEdit->setPlaceholderText(tr("User name"));
    depthSpin->setPrefix(tr("Depth: "));
    depthSpin->setToolTip(tr("Search depth: 1 finds direct matches, higher values follow alts of alts."));
    investigateButton->setText(tr("Investigate"));
    backButton->setText(tr("Back"));
    backButton->setToolTip(tr("Back to the previously investigated user"));
    if (currentUser.isEmpty()) {
        statusLabel->setText(tr("Enter a user name to investigate."));
    }

    emailChip->setText(tr("Email"));
    emailChip->setToolTip(tr("Show or hide email matches"));
    clientIdChip->setText(tr("Client ID"));
    clientIdChip->setToolTip(tr("Show or hide client ID matches"));
    ipChip->setText(tr("IP"));
    ipChip->setToolTip(tr("Show or hide IP address matches"));
    vpnLabel->setText(tr("Hide IPs shared by more than"));
    vpnSpin->setSuffix(tr(" users"));

    altsTable->setHorizontalHeaderLabels({tr("User"), tr("Via"), tr("Depth"), tr("Shared"), tr("eMail"),
                                          tr("Client ID"), tr("Registered"), tr("Last login"), tr("Warns"), tr("Bans"),
                                          tr("Active")});
    altsTable->setToolTip(tr("Double-click or press Enter on a row to investigate that account"));
    warnsTable->setHorizontalHeaderLabels({tr("Warning time"), tr("Moderator"), tr("Reason")});
    bansTable->setHorizontalHeaderLabels({tr("Ban time"), tr("Moderator"), tr("Length"), tr("Reason")});

    detailsGroup->setTitle(tr("Selected user"));
    detailNameLabel->setText(tr("Name:"));
    detailCountsLabel->setText(tr("Counts:"));
    detailNotesLabel->setText(tr("Admin notes:"));

    if (!lastAlts.isEmpty()) {
        renderTable();
    }
}

QString TabAlts::formatEpoch(quint64 ts) const
{
    if (ts == 0) {
        return tr("Unknown");
    }
    return QDateTime::fromSecsSinceEpoch(ts).toLocalTime().toString("yyyy-MM-dd HH:mm");
}

void TabAlts::investigate(const QString &userName)
{
    if (userName.isEmpty() || userName == currentUser) {
        return;
    }
    if (!currentUser.isEmpty()) {
        backStack << currentUser;
        backButton->setEnabled(true);
    }
    loadUser(userName);
}

void TabAlts::goBack()
{
    if (backStack.isEmpty()) {
        return;
    }
    const QString previousUser = backStack.takeLast();
    backButton->setEnabled(!backStack.isEmpty());
    loadUser(previousUser);
}

void TabAlts::loadUser(const QString &userName)
{
    currentUser = userName;
    searchEdit->setText(userName);
    investigateUser();
}

void TabAlts::investigateUser()
{
    const QString userName = searchEdit->text().simplified();
    if (userName.isEmpty()) {
        return;
    }
    currentUser = userName;
    lastAlts.clear();
    selectedUser.clear();
    clearDetails();
    altsTable->setRowCount(0);
    statusLabel->setText(tr("Loading alts for %1...").arg(currentUser));
    requestAlts(userName, depthSpin->value());
}

void TabAlts::requestAlts(const QString &userName, int depth)
{
    Command_GetUserAlts cmd;
    cmd.set_user_name(userName.toStdString());
    cmd.set_depth(static_cast<google::protobuf::uint32>(depth));
    PendingCommand *pend = client->prepareModeratorCommand(cmd);
    connect(pend, &PendingCommand::finished, this, [this, userName](const Response &response) {
        if (userName == currentUser) {
            altsResponse(response);
        }
    });
    client->sendCommand(pend);
}

void TabAlts::altsResponse(const Response &response)
{
    lastAlts.clear();
    if (response.response_code() != Response::RespOk) {
        renderTable();
        statusLabel->setText(tr("User %1 not found.").arg(currentUser));
        return;
    }

    const Response_UserAlts &resp = response.GetExtension(Response_UserAlts::ext);
    for (int i = 0; i < resp.alts_size(); ++i) {
        lastAlts << resp.alts(i);
    }
    renderTable();
}

bool TabAlts::rowVisible(const ServerInfo_UserAlt &alt) const
{
    const QString name = QString::fromStdString(alt.user_name());
    if (name == currentUser) {
        return true;
    }
    if (!alt.has_match_reason() || alt.match_reason().empty()) {
        return true;
    }
    const QString reason = QString::fromStdString(alt.match_reason());
    if (reason == QStringLiteral("email")) {
        return emailChip->isChecked();
    }
    if (reason.startsWith(QStringLiteral("clientid:"))) {
        return clientIdChip->isChecked();
    }
    if (reason.startsWith(QStringLiteral("ip:"))) {
        return ipChip->isChecked() &&
               (!alt.has_shared_count() || alt.shared_count() <= static_cast<unsigned>(vpnSpin->value()));
    }
    return true;
}

void TabAlts::renderTable()
{
    // Blocked so the programmatic refill neither wipes the details panel
    // (via the selection-cleared path) nor refetches them on restore.
    const QSignalBlocker blocker(altsTable);
    altsTable->setRowCount(0);

    int emailCount = 0;
    int clientIdCount = 0;
    int ipCount = 0;
    for (const ServerInfo_UserAlt &alt : lastAlts) {
        const QString reason = QString::fromStdString(alt.match_reason());
        if (reason == QStringLiteral("email")) {
            ++emailCount;
        } else if (reason.startsWith(QStringLiteral("clientid:"))) {
            ++clientIdCount;
        } else if (reason.startsWith(QStringLiteral("ip:"))) {
            ++ipCount;
        }
    }
    emailChip->setText(tr("Email (%1)").arg(emailCount));
    clientIdChip->setText(tr("Client ID (%1)").arg(clientIdCount));
    ipChip->setText(tr("IP (%1)").arg(ipCount));

    QFont userFont = altsTable->font();
    userFont.setBold(true);
    const QColor dimColor = palette().color(QPalette::Disabled, QPalette::Text);

    int hidden = 0;
    int row = 0;
    for (const ServerInfo_UserAlt &alt : lastAlts) {
        if (!rowVisible(alt)) {
            ++hidden;
            continue;
        }
        altsTable->insertRow(row);
        auto *userItem = new QTableWidgetItem(QString::fromStdString(alt.user_name()));
        userItem->setFont(userFont);
        altsTable->setItem(row, COL_ALTS_USER, userItem);
        altsTable->setItem(row, COL_ALTS_VIA,
                           new QTableWidgetItem(alt.has_match_reason() && !alt.match_reason().empty()
                                                    ? QString::fromStdString(alt.match_reason())
                                                    : QStringLiteral("-")));
        altsTable->setItem(row, COL_ALTS_DEPTH,
                           new QTableWidgetItem(alt.has_depth() ? QString::number(alt.depth()) : QStringLiteral("-")));
        altsTable->setItem(
            row, COL_ALTS_SHARED,
            new QTableWidgetItem(alt.has_shared_count() ? QString::number(alt.shared_count()) : QStringLiteral("-")));
        altsTable->setItem(row, COL_ALTS_EMAIL, new QTableWidgetItem(QString::fromStdString(alt.email())));
        altsTable->setItem(row, COL_ALTS_CLIENTID, new QTableWidgetItem(QString::fromStdString(alt.clientid())));
        altsTable->setItem(row, COL_ALTS_REGISTERED, new QTableWidgetItem(formatEpoch(alt.registration_time())));
        altsTable->setItem(row, COL_ALTS_LAST_LOGIN, new QTableWidgetItem(formatEpoch(alt.last_login())));
        altsTable->setItem(row, COL_ALTS_WARNS, new QTableWidgetItem(QString::number(alt.warn_count())));
        altsTable->setItem(row, COL_ALTS_BANS, new QTableWidgetItem(QString::number(alt.ban_count())));
        altsTable->setItem(row, COL_ALTS_ACTIVE, new QTableWidgetItem(alt.is_active() ? tr("yes") : tr("no")));
        if (QString::fromStdString(alt.match_reason()).startsWith(QStringLiteral("ip:")) && alt.has_shared_count() &&
            alt.shared_count() > 2) {
            for (int col = 0; col < COL_ALTS_COUNT; ++col) {
                if (auto *item = altsTable->item(row, col)) {
                    item->setForeground(dimColor);
                }
            }
        }
        ++row;
    }
    if (hidden == 0) {
        statusLabel->setText(tr("%1 account(s) related to %2.").arg(lastAlts.size()).arg(currentUser));
    } else {
        statusLabel->setText(tr("%1 shown, %2 hidden by filters (%3 total) related to %4.")
                                 .arg(row)
                                 .arg(hidden)
                                 .arg(lastAlts.size())
                                 .arg(currentUser));
    }

    if (!selectedUser.isEmpty()) {
        bool restored = false;
        for (int restoreRow = 0; restoreRow < altsTable->rowCount(); ++restoreRow) {
            auto *userItem = altsTable->item(restoreRow, COL_ALTS_USER);
            if (userItem != nullptr && userItem->text() == selectedUser) {
                altsTable->setCurrentItem(userItem, QItemSelectionModel::Select | QItemSelectionModel::Rows);
                restored = true;
                break;
            }
        }
        if (!restored) {
            selectedUser.clear();
            clearDetails();
        }
    }
}

void TabAlts::onFilterChanged()
{
    if (lastAlts.isEmpty()) {
        return;
    }
    renderTable();
}

void TabAlts::onAltSelected()
{
    const auto selected = altsTable->selectedItems();
    if (selected.isEmpty()) {
        selectedUser.clear();
        clearDetails();
        return;
    }
    auto *userItem = altsTable->item(selected.first()->row(), COL_ALTS_USER);
    if (userItem == nullptr || userItem->text().isEmpty() || userItem->text() == selectedUser) {
        return;
    }
    selectedUser = userItem->text();
    requestDetails(selectedUser);
}

void TabAlts::requestDetails(const QString &userName)
{
    detailNameValue->setText(userName);
    detailCountsValue->setText(tr("Loading..."));
    detailNotesEdit->clear();
    warnsTable->setRowCount(0);
    bansTable->setRowCount(0);

    Command_ReportUserInfo infoCmd;
    infoCmd.set_user_name(userName.toStdString());
    PendingCommand *infoPend = client->prepareModeratorCommand(infoCmd);
    connect(infoPend, &PendingCommand::finished, this, [this, userName](const Response &response) {
        if (userName == selectedUser) {
            userDetailsResponse(response);
        }
    });
    client->sendCommand(infoPend);

    Command_GetWarnHistory warnCmd;
    warnCmd.set_user_name(userName.toStdString());
    PendingCommand *warnPend = client->prepareModeratorCommand(warnCmd);
    connect(warnPend, &PendingCommand::finished, this, [this, userName](const Response &response) {
        if (userName == selectedUser) {
            warnHistoryResponse(response);
        }
    });
    client->sendCommand(warnPend);

    Command_GetBanHistory banCmd;
    banCmd.set_user_name(userName.toStdString());
    PendingCommand *banPend = client->prepareModeratorCommand(banCmd);
    connect(banPend, &PendingCommand::finished, this, [this, userName](const Response &response) {
        if (userName == selectedUser) {
            banHistoryResponse(response);
        }
    });
    client->sendCommand(banPend);
}

void TabAlts::clearDetails()
{
    detailNameValue->clear();
    detailCountsValue->setText(tr("Select a row to inspect its history."));
    detailNotesEdit->clear();
    warnsTable->setRowCount(0);
    bansTable->setRowCount(0);
}

void TabAlts::userDetailsResponse(const Response &response)
{
    if (response.response_code() != Response::RespOk) {
        detailCountsValue->setText(tr("Error loading user info."));
        return;
    }

    const Response_ReportUserInfo &resp = response.GetExtension(Response_ReportUserInfo::ext);
    detailCountsValue->setText(tr("Reports: %1   Bans: %2   Warnings: %3")
                                   .arg(resp.total_reports())
                                   .arg(resp.total_bans())
                                   .arg(resp.total_warns()));

    if (resp.has_admin_notes() && !resp.admin_notes().empty()) {
        detailNotesEdit->setPlainText(QString::fromStdString(resp.admin_notes()));
    } else {
        detailNotesEdit->setPlainText(tr("(none)"));
    }
}

void TabAlts::warnHistoryResponse(const Response &response)
{
    warnsTable->setRowCount(0);
    if (response.response_code() != Response::RespOk) {
        return;
    }

    const Response_WarnHistory &resp = response.GetExtension(Response_WarnHistory::ext);
    warnsTable->setRowCount(resp.warn_list_size());
    for (int i = 0; i < resp.warn_list_size(); ++i) {
        const ServerInfo_Warning &warn = resp.warn_list(i);
        warnsTable->setItem(i, COL_WARNS_TIME, new QTableWidgetItem(QString::fromStdString(warn.time_of())));
        warnsTable->setItem(i, COL_WARNS_MODERATOR, new QTableWidgetItem(QString::fromStdString(warn.admin_name())));
        warnsTable->setItem(i, COL_WARNS_REASON, new QTableWidgetItem(QString::fromStdString(warn.reason())));
    }
}

void TabAlts::banHistoryResponse(const Response &response)
{
    bansTable->setRowCount(0);
    if (response.response_code() != Response::RespOk) {
        return;
    }

    const Response_BanHistory &resp = response.GetExtension(Response_BanHistory::ext);
    bansTable->setRowCount(resp.ban_list_size());
    for (int i = 0; i < resp.ban_list_size(); ++i) {
        const ServerInfo_Ban &ban = resp.ban_list(i);
        bansTable->setItem(i, COL_BANS_TIME, new QTableWidgetItem(QString::fromStdString(ban.ban_time())));
        bansTable->setItem(i, COL_BANS_MODERATOR, new QTableWidgetItem(QString::fromStdString(ban.admin_name())));
        bansTable->setItem(i, COL_BANS_LENGTH, new QTableWidgetItem(QString::fromStdString(ban.ban_length())));
        bansTable->setItem(i, COL_BANS_REASON, new QTableWidgetItem(QString::fromStdString(ban.ban_reason())));
    }
}
