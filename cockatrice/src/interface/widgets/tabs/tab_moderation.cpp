#include "tab_moderation.h"

#include "../utility/table_activation.h"
#include "abstract_client.h"
#include "libcockatrice/protocol/pb/admin_commands.pb.h"
#include "libcockatrice/protocol/pb/response.pb.h"
#include "libcockatrice/protocol/pb/serverinfo_moderator_login.pb.h"
#include "libcockatrice/protocol/pb/serverinfo_user_alt.pb.h"
#include "libcockatrice/protocol/pb/serverinfo_user_session.pb.h"
#include "tab_supervisor.h"

#include <QAbstractItemView>
#include <QDateTime>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QList>
#include <QMessageBox>
#include <QPushButton>
#include <QSharedPointer>
#include <QSplitter>
#include <QStringList>
#include <QTabWidget>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QWidget>
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
#include <qnamespace.h>
#include <string>

class TabSupervisor;

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
    userInfoNameValue = new QLabel;
    QFont displayFont = userInfoNameValue->font();
    displayFont.setPointSize(displayFont.pointSize() + 6);
    displayFont.setBold(true);
    userInfoNameValue->setFont(displayFont);
    userInfoStatusValue = new QLabel;
    QFont statusFont = userInfoStatusValue->font();
    statusFont.setBold(true);
    userInfoStatusValue->setFont(statusFont);
    sublineLabel = new QLabel;

    auto makeCountCard = [](const QString &title, QLabel *&value) {
        auto *card = new QGroupBox(title);
        value = new QLabel(QStringLiteral("–"), card);
        QFont cardFont = value->font();
        cardFont.setPointSize(cardFont.pointSize() + 6);
        cardFont.setBold(true);
        value->setFont(cardFont);
        value->setAlignment(Qt::AlignCenter);
        auto *cardLayout = new QVBoxLayout(card);
        cardLayout->addWidget(value);
        return card;
    };
    cardReports = makeCountCard(QString(), cardReportsValue);
    cardWarns = makeCountCard(QString(), cardWarnsValue);
    cardBans = makeCountCard(QString(), cardBansValue);

    altsCard = new QGroupBox;
    altsCountValue = new QLabel(QStringLiteral("–"), altsCard);
    QFont altsFont = altsCountValue->font();
    altsFont.setPointSize(altsFont.pointSize() + 6);
    altsFont.setBold(true);
    altsCountValue->setFont(altsFont);
    altsCountValue->setAlignment(Qt::AlignCenter);
    openAltsButton = new QPushButton;
    connect(openAltsButton, &QPushButton::clicked, this, &TabModeration::openAltsGraph);
    auto *altsCardLayout = new QVBoxLayout(altsCard);
    altsCardLayout->addWidget(altsCountValue);
    altsCardLayout->addWidget(openAltsButton);

    auto *cardsRow = new QHBoxLayout;
    cardsRow->addWidget(cardReports);
    cardsRow->addWidget(cardWarns);
    cardsRow->addWidget(cardBans);
    cardsRow->addWidget(altsCard);

    userInfoNotesLabel = new QLabel;
    userInfoNotesEdit = new QTextEdit;
    userInfoNotesEdit->setReadOnly(true);

    auto *identityRow = new QHBoxLayout;
    identityRow->addWidget(userInfoNameValue);
    identityRow->addStretch();
    identityRow->addWidget(userInfoStatusValue);

    auto *infoLayout = new QVBoxLayout(userInfoGroup);
    infoLayout->addLayout(identityRow);
    infoLayout->addWidget(sublineLabel);
    infoLayout->addLayout(cardsRow);
    infoLayout->addWidget(userInfoNotesLabel);
    infoLayout->addWidget(userInfoNotesEdit);

    auto configureTable = [](QTableWidget *table) {
        table->setSelectionBehavior(QAbstractItemView::SelectRows);
        table->setSelectionMode(QAbstractItemView::SingleSelection);
        table->setEditTriggers(QAbstractItemView::NoEditTriggers);
        table->verticalHeader()->setVisible(false);
        table->setAlternatingRowColors(true);
        table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
        table->horizontalHeader()->setStretchLastSection(true);
    };

    altsTable = new QTableWidget(0, COL_ALTS_COUNT);
    configureTable(altsTable);
    connectTableActivation(altsTable, this, [this](const QModelIndex &index) {
        if (!index.isValid()) {
            return;
        }
        auto *userItem = altsTable->item(index.row(), COL_ALTS_USER);
        if (userItem != nullptr && !userItem->text().isEmpty()) {
            getTabSupervisor()->openTabAlts(userItem->text());
        }
    });
    auto *altsPage = new QWidget;
    auto *altsLayout = new QVBoxLayout(altsPage);
    altsLayout->addWidget(altsTable);

    sessionsTable = new QTableWidget(0, COL_SESSIONS_COUNT);
    configureTable(sessionsTable);
    auto *sessionsPage = new QWidget;
    auto *sessionsLayout = new QVBoxLayout(sessionsPage);
    sessionsLayout->addWidget(sessionsTable);

    staffTable = new QTableWidget(0, COL_STAFF_COUNT);
    configureTable(staffTable);
    refreshStaffButton = new QPushButton;
    connect(refreshStaffButton, &QPushButton::clicked, this, &TabModeration::requestModeratorLogins);
    auto *staffHeader = new QHBoxLayout;
    staffHeader->addStretch();
    staffHeader->addWidget(refreshStaffButton);
    auto *staffPage = new QWidget;
    auto *staffLayout = new QVBoxLayout(staffPage);
    staffLayout->addWidget(staffTable);
    staffLayout->addLayout(staffHeader);

    sectionTabs = new QTabWidget;
    sectionTabs->addTab(altsPage, QString());
    sectionTabs->addTab(sessionsPage, QString());
    sectionTabs->addTab(staffPage, QString());

    auto *splitter = new QSplitter(Qt::Vertical);
    splitter->addWidget(userInfoGroup);
    splitter->addWidget(sectionTabs);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 2);

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
    userInfoNotesLabel->setText(tr("Admin notes:"));
    cardReports->setTitle(tr("Reports"));
    cardWarns->setTitle(tr("Warnings"));
    cardBans->setTitle(tr("Bans"));
    altsCard->setTitle(tr("Alts"));
    openAltsButton->setText(tr("Open graph"));
    openAltsButton->setToolTip(tr("Open this user in the User Alts tab"));

    sectionTabs->setTabText(0, tr("Alts"));
    sectionTabs->setTabText(1, tr("Sessions"));
    sectionTabs->setTabText(2, tr("Staff Last Logins"));

    if (cachedUserInfoValid) {
        applyUserInfo();
    }
    if (cachedAltsCount >= 0) {
        applyAltsCount();
    }

    altsTable->setHorizontalHeaderLabels({tr("User"), tr("eMail"), tr("Client ID"), tr("Registered"), tr("Last login"),
                                          tr("Warns"), tr("Bans"), tr("Active")});
    altsTable->setToolTip(tr("Double-click or press Enter on a row to open it in the User Alts tab"));
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
    cachedUserInfoValid = false;
    cachedAltsCount = -1;
    userInfoNameValue->clear();
    userInfoStatusValue->clear();
    sublineLabel->clear();
    cardReportsValue->setText(QStringLiteral("–"));
    cardWarnsValue->setText(QStringLiteral("–"));
    cardBansValue->setText(QStringLiteral("–"));
    altsCountValue->setText(QStringLiteral("–"));
    userInfoNotesEdit->clear();
    altsTable->setRowCount(0);
    sessionsTable->setRowCount(0);
    resetPasswordButton->setEnabled(false);
    removeAvatarButton->setEnabled(false);
    openAltsButton->setEnabled(false);
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
    cachedUserInfoValid = false;
    cachedAltsCount = -1;
    resetPasswordButton->setEnabled(true);
    removeAvatarButton->setEnabled(true);
    openAltsButton->setEnabled(true);
    altsCountValue->setText(QStringLiteral("…"));
    altsTable->setRowCount(0);
    sessionsTable->setRowCount(0);
    requestUserInfo(userName);
    requestSessions(userName);
    requestAlts(userName);
}

void TabModeration::openAltsGraph()
{
    if (currentUser.isEmpty()) {
        return;
    }
    getTabSupervisor()->openTabAlts(currentUser);
}

void TabModeration::requestUserInfo(const QString &userName)
{
    userInfoNameValue->setText(userName);
    sublineLabel->setText(tr("Loading..."));
    userInfoStatusValue->clear();
    cardReportsValue->setText(QStringLiteral("…"));
    cardWarnsValue->setText(QStringLiteral("…"));
    cardBansValue->setText(QStringLiteral("…"));
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
        cachedUserInfoValid = false;
        sublineLabel->clear();
        userInfoStatusValue->setText(tr("Error loading user info."));
        cardReportsValue->setText(QStringLiteral("–"));
        cardWarnsValue->setText(QStringLiteral("–"));
        cardBansValue->setText(QStringLiteral("–"));
        return;
    }

    const Response_ReportUserInfo &resp = response.GetExtension(Response_ReportUserInfo::ext);
    if (resp.has_user_name() && QString::fromStdString(resp.user_name()) != currentUser) {
        return;
    }

    cachedUserInfo.CopyFrom(resp);
    cachedUserInfoValid = true;
    applyUserInfo();
}

void TabModeration::applyUserInfo()
{
    const Response_ReportUserInfo &resp = cachedUserInfo;
    sublineLabel->setText(tr("Registered %1 · Last login %2")
                              .arg(formatEpoch(resp.registration_time()))
                              .arg(formatEpoch(resp.last_login())));

    QStringList statusParts;
    statusParts << (resp.is_active() ? tr("active") : tr("inactive"));
    if (resp.has_is_admin() && resp.is_admin()) {
        statusParts << tr("admin");
    }
    userInfoStatusValue->setText(statusParts.join(QStringLiteral(", ")));

    cardReportsValue->setText(QString::number(resp.total_reports()));
    cardWarnsValue->setText(QString::number(resp.total_warns()));
    cardBansValue->setText(QString::number(resp.total_bans()));

    if (resp.has_admin_notes() && !resp.admin_notes().empty()) {
        userInfoNotesEdit->setPlainText(QString::fromStdString(resp.admin_notes()));
    } else {
        userInfoNotesEdit->setPlainText(tr("(none)"));
    }
}

void TabModeration::applyAltsCount()
{
    altsCountValue->setText(tr("%1 related").arg(cachedAltsCount));
}

void TabModeration::requestSessions(const QString &userName)
{
    Command_GetUserSessions cmd;
    cmd.set_user_name(userName.toStdString());
    PendingCommand *pend = client->prepareModeratorCommand(cmd);
    connect(pend, &PendingCommand::finished, this, [this, userName](const Response &response) {
        if (userName == currentUser) {
            sessionsResponse(response);
        }
    });
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
    connect(pend, &PendingCommand::finished, this, [this, userName](const Response &response) {
        if (userName == currentUser) {
            altsResponse(response);
        }
    });
    client->sendCommand(pend);
}

void TabModeration::altsResponse(const Response &response)
{
    altsTable->setRowCount(0);
    if (response.response_code() != Response::RespOk) {
        cachedAltsCount = -1;
        altsCountValue->setText(QStringLiteral("–"));
        return;
    }

    const Response_UserAlts &resp = response.GetExtension(Response_UserAlts::ext);
    // The server always includes the frontier user as a depth-0 seed row.
    int relatedCount = 0;
    for (int i = 0; i < resp.alts_size(); ++i) {
        if (!resp.alts(i).has_depth() || resp.alts(i).depth() > 0) {
            ++relatedCount;
        }
    }
    cachedAltsCount = relatedCount;
    applyAltsCount();
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
