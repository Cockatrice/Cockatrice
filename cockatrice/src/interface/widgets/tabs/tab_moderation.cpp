#include "tab_moderation.h"

#include "abstract_client.h"
#include "tab_supervisor.h"

#include <QDateTime>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSplitter>
#include <QTableWidget>
#include <QTextEdit>
#include <QVBoxLayout>
#include <libcockatrice/protocol/pb/command_report_user_info.pb.h>
#include <libcockatrice/protocol/pb/moderator_commands.pb.h>
#include <libcockatrice/protocol/pb/response_moderator_last_logins.pb.h>
#include <libcockatrice/protocol/pb/response_remove_user_avatar.pb.h>
#include <libcockatrice/protocol/pb/response_report_user_info.pb.h>
#include <libcockatrice/protocol/pb/response_reset_user_password.pb.h>
#include <libcockatrice/protocol/pb/response_user_alts.pb.h>
#include <libcockatrice/protocol/pb/response_user_sessions.pb.h>
#include <libcockatrice/protocol/pb/serverinfo_user.pb.h>
#include <libcockatrice/protocol/pending_command.h>

namespace
{
constexpr int COL_ALTS_USER = 0;
constexpr int COL_ALTS_EMAIL = 1;
constexpr int COL_ALTS_CLIENTID = 2;
constexpr int COL_ALTS_REGISTERED = 3;
constexpr int COL_ALTS_LAST_LOGIN = 4;
constexpr int COL_ALTS_WARNS = 5;
constexpr int COL_ALTS_BANS = 6;
constexpr int COL_ALTS_ACTIVE = 7;
constexpr int COL_ALTS_COUNT = 8;

constexpr int COL_SESSIONS_IP = 0;
constexpr int COL_SESSIONS_CLIENTID = 1;
constexpr int COL_SESSIONS_START = 2;
constexpr int COL_SESSIONS_END = 3;
constexpr int COL_SESSIONS_TYPE = 4;
constexpr int COL_SESSIONS_COUNT = 5;

constexpr int COL_STAFF_USER = 0;
constexpr int COL_STAFF_LEVEL = 1;
constexpr int COL_STAFF_LAST_LOGIN = 2;
constexpr int COL_STAFF_COUNT = 3;
} // namespace

TabModeration::TabModeration(TabSupervisor *_tabSupervisor, AbstractClient *_client, const QString &initialUser)
    : Tab(_tabSupervisor), client(_client)
{
    auto *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    searchEdit = new QLineEdit;
    searchEdit->setClearButtonEnabled(true);
    connect(searchEdit, &QLineEdit::returnPressed, this, &TabModeration::investigateUser);

    investigateButton = new QPushButton;
    connect(investigateButton, &QPushButton::clicked, this, &TabModeration::investigateUser);

    resetPasswordButton = new QPushButton;
    connect(resetPasswordButton, &QPushButton::clicked, this, &TabModeration::resetPassword);

    removeAvatarButton = new QPushButton;
    connect(removeAvatarButton, &QPushButton::clicked, this, &TabModeration::removeAvatar);

    auto *topBar = new QHBoxLayout;
    topBar->addWidget(searchEdit);
    topBar->addWidget(investigateButton);
    topBar->addStretch();
    topBar->addWidget(resetPasswordButton);
    topBar->addWidget(removeAvatarButton);

    userInfoGroup = new QGroupBox;
    userInfoNameLabel = new QLabel;
    userInfoNameValue = new QLabel;
    userInfoRegisteredLabel = new QLabel;
    userInfoRegisteredValue = new QLabel;
    userInfoLastLoginLabel = new QLabel;
    userInfoLastLoginValue = new QLabel;
    userInfoStatusLabel = new QLabel;
    userInfoStatusValue = new QLabel;
    userInfoCountsLabel = new QLabel;
    userInfoCountsValue = new QLabel;
    userInfoNotesLabel = new QLabel;
    userInfoNotesEdit = new QTextEdit;
    userInfoNotesEdit->setReadOnly(true);

    auto *infoGrid = new QGridLayout;
    infoGrid->addWidget(userInfoNameLabel, 0, 0);
    infoGrid->addWidget(userInfoNameValue, 0, 1);
    infoGrid->addWidget(userInfoRegisteredLabel, 0, 2);
    infoGrid->addWidget(userInfoRegisteredValue, 0, 3);
    infoGrid->addWidget(userInfoLastLoginLabel, 1, 0);
    infoGrid->addWidget(userInfoLastLoginValue, 1, 1);
    infoGrid->addWidget(userInfoStatusLabel, 1, 2);
    infoGrid->addWidget(userInfoStatusValue, 1, 3);
    infoGrid->addWidget(userInfoCountsLabel, 2, 0);
    infoGrid->addWidget(userInfoCountsValue, 2, 1, 1, 3);
    infoGrid->addWidget(userInfoNotesLabel, 3, 0, Qt::AlignTop);
    infoGrid->addWidget(userInfoNotesEdit, 3, 1, 1, 3);
    infoGrid->setColumnStretch(1, 1);
    infoGrid->setColumnStretch(3, 1);

    auto *infoLayout = new QVBoxLayout(userInfoGroup);
    infoLayout->addLayout(infoGrid);

    auto configureTable = [](QTableWidget *table) {
        table->setSelectionBehavior(QAbstractItemView::SelectRows);
        table->setSelectionMode(QAbstractItemView::SingleSelection);
        table->setEditTriggers(QAbstractItemView::NoEditTriggers);
        table->verticalHeader()->setVisible(false);
        table->setAlternatingRowColors(true);
        table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
        table->horizontalHeader()->setStretchLastSection(true);
    };

    altsGroup = new QGroupBox;
    altsTable = new QTableWidget(0, COL_ALTS_COUNT);
    configureTable(altsTable);
    auto *altsLayout = new QVBoxLayout(altsGroup);
    altsLayout->addWidget(altsTable);

    sessionsGroup = new QGroupBox;
    sessionsTable = new QTableWidget(0, COL_SESSIONS_COUNT);
    configureTable(sessionsTable);
    auto *sessionsLayout = new QVBoxLayout(sessionsGroup);
    sessionsLayout->addWidget(sessionsTable);

    staffGroup = new QGroupBox;
    staffTable = new QTableWidget(0, COL_STAFF_COUNT);
    configureTable(staffTable);
    refreshStaffButton = new QPushButton;
    connect(refreshStaffButton, &QPushButton::clicked, this, &TabModeration::requestModeratorLogins);
    auto *staffHeader = new QHBoxLayout;
    staffHeader->addStretch();
    staffHeader->addWidget(refreshStaffButton);
    auto *staffLayout = new QVBoxLayout(staffGroup);
    staffLayout->addWidget(staffTable);
    staffLayout->addLayout(staffHeader);

    auto *splitter = new QSplitter(Qt::Vertical);
    splitter->addWidget(userInfoGroup);
    splitter->addWidget(altsGroup);
    splitter->addWidget(sessionsGroup);
    splitter->addWidget(staffGroup);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 2);
    splitter->setStretchFactor(2, 2);
    splitter->setStretchFactor(3, 1);

    auto *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->addLayout(topBar);
    mainLayout->addWidget(splitter);

    retranslateUi();
    clearUserData();
    requestModeratorLogins();
    investigate(initialUser);
}

void TabModeration::retranslateUi()
{
    searchEdit->setPlaceholderText(tr("User name"));
    investigateButton->setText(tr("Investigate"));
    resetPasswordButton->setText(tr("Reset Password"));
    removeAvatarButton->setText(tr("Remove Avatar"));
    refreshStaffButton->setText(tr("Refresh"));

    userInfoGroup->setTitle(tr("User Info"));
    userInfoNameLabel->setText(tr("Name:"));
    userInfoRegisteredLabel->setText(tr("Registered:"));
    userInfoLastLoginLabel->setText(tr("Last login:"));
    userInfoStatusLabel->setText(tr("Status:"));
    userInfoCountsLabel->setText(tr("Counts:"));
    userInfoNotesLabel->setText(tr("Admin notes:"));

    altsGroup->setTitle(tr("Alts"));
    sessionsGroup->setTitle(tr("Sessions"));
    staffGroup->setTitle(tr("Staff Last Logins"));

    altsTable->setHorizontalHeaderLabels({tr("User"), tr("eMail"), tr("Client ID"), tr("Registered"), tr("Last login"),
                                          tr("Warns"), tr("Bans"), tr("Active")});
    sessionsTable->setHorizontalHeaderLabels({tr("IP"), tr("Client ID"), tr("Start"), tr("End"), tr("Type")});
    staffTable->setHorizontalHeaderLabels({tr("User"), tr("Level"), tr("Last login")});
}

QString TabModeration::formatEpoch(quint64 ts) const
{
    if (ts == 0) {
        return tr("Unknown");
    }
    return QDateTime::fromSecsSinceEpoch(ts).toLocalTime().toString("yyyy-MM-dd HH:mm");
}

void TabModeration::clearUserData()
{
    currentUser.clear();
    userInfoNameValue->clear();
    userInfoRegisteredValue->clear();
    userInfoLastLoginValue->clear();
    userInfoStatusValue->clear();
    userInfoCountsValue->clear();
    userInfoNotesEdit->clear();
    altsTable->setRowCount(0);
    sessionsTable->setRowCount(0);
    resetPasswordButton->setEnabled(false);
    removeAvatarButton->setEnabled(false);
}

void TabModeration::investigate(const QString &userName)
{
    if (userName.isEmpty()) {
        return;
    }
    searchEdit->setText(userName);
    investigateUser();
}

void TabModeration::investigateUser()
{
    const QString userName = searchEdit->text().simplified();
    if (userName.isEmpty()) {
        return;
    }
    currentUser = userName;
    resetPasswordButton->setEnabled(true);
    removeAvatarButton->setEnabled(true);
    altsTable->setRowCount(0);
    sessionsTable->setRowCount(0);
    requestUserInfo(userName);
    requestSessions(userName);
    requestAlts(userName);
}

void TabModeration::requestUserInfo(const QString &userName)
{
    userInfoNameValue->setText(userName);
    userInfoRegisteredValue->setText(tr("Loading..."));
    userInfoLastLoginValue->setText(tr("Loading..."));
    userInfoStatusValue->clear();
    userInfoCountsValue->clear();
    userInfoNotesEdit->clear();

    Command_ReportUserInfo cmd;
    cmd.set_user_name(userName.toStdString());
    PendingCommand *pend = client->prepareModeratorCommand(cmd);
    connect(pend, &PendingCommand::finished, this, &TabModeration::userInfoResponse);
    client->sendCommand(pend);
}

void TabModeration::userInfoResponse(const Response &response)
{
    if (response.response_code() != Response::RespOk) {
        userInfoRegisteredValue->clear();
        userInfoLastLoginValue->clear();
        userInfoStatusValue->setText(tr("Error loading user info."));
        return;
    }

    const Response_ReportUserInfo &resp = response.GetExtension(Response_ReportUserInfo::ext);
    if (resp.has_user_name() && QString::fromStdString(resp.user_name()) != currentUser) {
        return;
    }

    userInfoRegisteredValue->setText(formatEpoch(resp.registration_time()));
    userInfoLastLoginValue->setText(formatEpoch(resp.last_login()));

    QStringList statusParts;
    statusParts << (resp.is_active() ? tr("active") : tr("inactive"));
    if (resp.has_is_admin() && resp.is_admin()) {
        statusParts << tr("admin");
    }
    userInfoStatusValue->setText(statusParts.join(", "));

    userInfoCountsValue->setText(tr("Reports: %1   Bans: %2   Warnings: %3")
                                     .arg(resp.total_reports())
                                     .arg(resp.total_bans())
                                     .arg(resp.total_warns()));

    if (resp.has_admin_notes() && !resp.admin_notes().empty()) {
        userInfoNotesEdit->setPlainText(QString::fromStdString(resp.admin_notes()));
    } else {
        userInfoNotesEdit->setPlainText(tr("(none)"));
    }
}

void TabModeration::requestSessions(const QString &userName)
{
    Command_GetUserSessions cmd;
    cmd.set_user_name(userName.toStdString());
    PendingCommand *pend = client->prepareModeratorCommand(cmd);
    connect(pend, &PendingCommand::finished, this, &TabModeration::sessionsResponse);
    client->sendCommand(pend);
}

void TabModeration::sessionsResponse(const Response &response)
{
    sessionsTable->setRowCount(0);
    if (response.response_code() != Response::RespOk) {
        return;
    }

    const Response_UserSessions &resp = response.GetExtension(Response_UserSessions::ext);
    sessionsTable->setRowCount(resp.sessions_size());
    for (int i = 0; i < resp.sessions_size(); ++i) {
        const ServerInfo_UserSession &session = resp.sessions(i);
        sessionsTable->setItem(i, COL_SESSIONS_IP, new QTableWidgetItem(QString::fromStdString(session.ip_address())));
        sessionsTable->setItem(i, COL_SESSIONS_CLIENTID,
                               new QTableWidgetItem(QString::fromStdString(session.clientid())));
        sessionsTable->setItem(i, COL_SESSIONS_START, new QTableWidgetItem(formatEpoch(session.start_time())));
        sessionsTable->setItem(
            i, COL_SESSIONS_END,
            new QTableWidgetItem(session.end_time() == 0 ? tr("Active") : formatEpoch(session.end_time())));
        sessionsTable->setItem(i, COL_SESSIONS_TYPE,
                               new QTableWidgetItem(QString::fromStdString(session.connection_type())));
    }
}

void TabModeration::requestAlts(const QString &userName)
{
    Command_GetUserAlts cmd;
    cmd.set_user_name(userName.toStdString());
    PendingCommand *pend = client->prepareModeratorCommand(cmd);
    connect(pend, &PendingCommand::finished, this, &TabModeration::altsResponse);
    client->sendCommand(pend);
}

void TabModeration::altsResponse(const Response &response)
{
    altsTable->setRowCount(0);
    if (response.response_code() != Response::RespOk) {
        return;
    }

    const Response_UserAlts &resp = response.GetExtension(Response_UserAlts::ext);
    altsTable->setRowCount(resp.alts_size());
    for (int i = 0; i < resp.alts_size(); ++i) {
        const ServerInfo_UserAlt &alt = resp.alts(i);
        altsTable->setItem(i, COL_ALTS_USER, new QTableWidgetItem(QString::fromStdString(alt.user_name())));
        altsTable->setItem(i, COL_ALTS_EMAIL, new QTableWidgetItem(QString::fromStdString(alt.email())));
        altsTable->setItem(i, COL_ALTS_CLIENTID, new QTableWidgetItem(QString::fromStdString(alt.clientid())));
        altsTable->setItem(i, COL_ALTS_REGISTERED, new QTableWidgetItem(formatEpoch(alt.registration_time())));
        altsTable->setItem(i, COL_ALTS_LAST_LOGIN, new QTableWidgetItem(formatEpoch(alt.last_login())));
        altsTable->setItem(i, COL_ALTS_WARNS, new QTableWidgetItem(QString::number(alt.warn_count())));
        altsTable->setItem(i, COL_ALTS_BANS, new QTableWidgetItem(QString::number(alt.ban_count())));
        altsTable->setItem(i, COL_ALTS_ACTIVE, new QTableWidgetItem(alt.is_active() ? tr("yes") : tr("no")));
    }
}

void TabModeration::requestModeratorLogins()
{
    Command_GetModeratorLastLogins cmd;
    PendingCommand *pend = client->prepareModeratorCommand(cmd);
    connect(pend, &PendingCommand::finished, this, &TabModeration::moderatorLoginsResponse);
    client->sendCommand(pend);
}

void TabModeration::moderatorLoginsResponse(const Response &response)
{
    staffTable->setRowCount(0);
    if (response.response_code() != Response::RespOk) {
        return;
    }

    const Response_ModeratorLastLogins &resp = response.GetExtension(Response_ModeratorLastLogins::ext);
    staffTable->setRowCount(resp.logins_size());
    for (int i = 0; i < resp.logins_size(); ++i) {
        const ServerInfo_ModeratorLogin &login = resp.logins(i);
        staffTable->setItem(i, COL_STAFF_USER, new QTableWidgetItem(QString::fromStdString(login.user_name())));
        staffTable->setItem(i, COL_STAFF_LAST_LOGIN, new QTableWidgetItem(formatEpoch(login.last_login())));

        QStringList levels;
        if (login.user_level() & ServerInfo_User::IsAdmin) {
            levels << tr("Admin");
        }
        if (login.user_level() & ServerInfo_User::IsDeveloper) {
            levels << tr("Developer");
        }
        if (login.user_level() & ServerInfo_User::IsModerator) {
            levels << tr("Moderator");
        }
        if (login.user_level() & ServerInfo_User::IsJudge) {
            levels << tr("Judge");
        }
        staffTable->setItem(i, COL_STAFF_LEVEL, new QTableWidgetItem(levels.join(" / ")));
    }
}

void TabModeration::resetPassword()
{
    if (currentUser.isEmpty()) {
        return;
    }

    QMessageBox::StandardButton choice =
        QMessageBox::warning(this, tr("Reset Password"),
                             tr("Reset the password of %1? A temporary password will be generated and shown to you. "
                                "The user must change it on their first login.")
                                 .arg(currentUser),
                             QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel);
    if (choice != QMessageBox::Ok) {
        return;
    }

    Command_ResetUserPassword cmd;
    cmd.set_user_name(currentUser.toStdString());
    PendingCommand *pend = client->prepareModeratorCommand(cmd);
    connect(pend, &PendingCommand::finished, this, &TabModeration::resetPasswordResponse);
    client->sendCommand(pend);
}

void TabModeration::resetPasswordResponse(const Response &response)
{
    if (response.response_code() != Response::RespOk) {
        QMessageBox::critical(this, tr("Error"), tr("Password reset failed."));
        return;
    }

    const Response_ResetUserPassword &resp = response.GetExtension(Response_ResetUserPassword::ext);
    QMessageBox::information(this, tr("Password reset"),
                             tr("Temporary password for %1:\n\n%2\n\nPass it to the user through a secure channel.")
                                 .arg(QString::fromStdString(resp.user_name()))
                                 .arg(QString::fromStdString(resp.temporary_password())));
}

void TabModeration::removeAvatar()
{
    if (currentUser.isEmpty()) {
        return;
    }

    QMessageBox::StandardButton choice =
        QMessageBox::warning(this, tr("Remove Avatar"),
                             tr("Remove the avatar of %1? The user will have to upload a new one.").arg(currentUser),
                             QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel);
    if (choice != QMessageBox::Ok) {
        return;
    }

    Command_RemoveUserAvatar cmd;
    cmd.set_user_name(currentUser.toStdString());
    PendingCommand *pend = client->prepareModeratorCommand(cmd);
    connect(pend, &PendingCommand::finished, this, &TabModeration::removeAvatarResponse);
    client->sendCommand(pend);
}

void TabModeration::removeAvatarResponse(const Response &response)
{
    if (response.response_code() != Response::RespOk) {
        QMessageBox::critical(this, tr("Error"), tr("Could not remove the avatar."));
        return;
    }

    const Response_RemoveUserAvatar &resp = response.GetExtension(Response_RemoveUserAvatar::ext);
    QMessageBox::information(this, tr("Avatar removed"),
                             tr("The avatar of %1 has been removed.").arg(QString::fromStdString(resp.user_name())));
}
