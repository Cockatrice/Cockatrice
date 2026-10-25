#include "tab_alts.h"

#include "abstract_client.h"
#include "libcockatrice/protocol/pb/response.pb.h"
#include "libcockatrice/protocol/pb/response_user_alts.pb.h"
#include "libcockatrice/protocol/pb/serverinfo_user_alt.pb.h"

#include <QAbstractItemView>
#include <QDateTime>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>
#include <QWidget>
#include <libcockatrice/protocol/pb/moderator_commands.pb.h>
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

constexpr int MAX_ALT_DEPTH = 3;
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

    altsTable = new QTableWidget(0, COL_ALTS_COUNT);
    altsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    altsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    altsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    altsTable->verticalHeader()->setVisible(false);
    altsTable->setAlternatingRowColors(true);
    altsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    altsTable->horizontalHeader()->setStretchLastSection(true);
    connect(altsTable, &QTableWidget::cellDoubleClicked, this, [this](int row, int) {
        auto *userItem = altsTable->item(row, COL_ALTS_USER);
        if (userItem != nullptr && !userItem->text().isEmpty() && userItem->text() != currentUser) {
            investigate(userItem->text());
        }
    });

    auto *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->addLayout(topBar);
    mainLayout->addWidget(statusLabel);
    mainLayout->addWidget(altsTable);

    retranslateUi();
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

    altsTable->setHorizontalHeaderLabels({tr("User"), tr("Via"), tr("Depth"), tr("Shared"), tr("eMail"),
                                          tr("Client ID"), tr("Registered"), tr("Last login"), tr("Warns"), tr("Bans"),
                                          tr("Active")});
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
    connect(pend, &PendingCommand::finished, this, &TabAlts::altsResponse);
    client->sendCommand(pend);
}

void TabAlts::altsResponse(const Response &response)
{
    altsTable->setRowCount(0);
    if (response.response_code() != Response::RespOk) {
        statusLabel->setText(tr("User %1 not found.").arg(currentUser));
        return;
    }

    const Response_UserAlts &resp = response.GetExtension(Response_UserAlts::ext);
    altsTable->setRowCount(resp.alts_size());
    for (int i = 0; i < resp.alts_size(); ++i) {
        const ServerInfo_UserAlt &alt = resp.alts(i);
        altsTable->setItem(i, COL_ALTS_USER, new QTableWidgetItem(QString::fromStdString(alt.user_name())));
        altsTable->setItem(i, COL_ALTS_VIA,
                           new QTableWidgetItem(alt.has_match_reason() && !alt.match_reason().empty()
                                                    ? QString::fromStdString(alt.match_reason())
                                                    : QStringLiteral("-")));
        altsTable->setItem(i, COL_ALTS_DEPTH,
                           new QTableWidgetItem(alt.has_depth() ? QString::number(alt.depth()) : QStringLiteral("-")));
        altsTable->setItem(
            i, COL_ALTS_SHARED,
            new QTableWidgetItem(alt.has_shared_count() ? QString::number(alt.shared_count()) : QStringLiteral("-")));
        altsTable->setItem(i, COL_ALTS_EMAIL, new QTableWidgetItem(QString::fromStdString(alt.email())));
        altsTable->setItem(i, COL_ALTS_CLIENTID, new QTableWidgetItem(QString::fromStdString(alt.clientid())));
        altsTable->setItem(i, COL_ALTS_REGISTERED, new QTableWidgetItem(formatEpoch(alt.registration_time())));
        altsTable->setItem(i, COL_ALTS_LAST_LOGIN, new QTableWidgetItem(formatEpoch(alt.last_login())));
        altsTable->setItem(i, COL_ALTS_WARNS, new QTableWidgetItem(QString::number(alt.warn_count())));
        altsTable->setItem(i, COL_ALTS_BANS, new QTableWidgetItem(QString::number(alt.ban_count())));
        altsTable->setItem(i, COL_ALTS_ACTIVE, new QTableWidgetItem(alt.is_active() ? tr("yes") : tr("no")));
    }
    statusLabel->setText(tr("%1 account(s) related to %2.").arg(resp.alts_size()).arg(currentUser));
}
