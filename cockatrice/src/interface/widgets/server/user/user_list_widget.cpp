#include "user_list_widget.h"

#include "../../interface/pixel_map_generator.h"
#include "../../interface/widgets/tabs/tab_account.h"
#include "../../interface/widgets/tabs/tab_supervisor.h"
#include "../game_selector.h"
#include "user_context_menu.h"

#include <QApplication>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QWidget>
#include <libcockatrice/network/client/abstract/abstract_client.h>
#include <libcockatrice/protocol/pb/response_get_games_of_user.pb.h>
#include <libcockatrice/utility/trice_limits.h>

BanDialog::BanDialog(const ServerInfo_User &info, QWidget *parent) : QDialog(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);

    nameBanCheckBox = new QCheckBox(tr("ban &user name"));
    nameBanCheckBox->setChecked(true);
    nameBanEdit = new QLineEdit(QString::fromStdString(info.name()));
    nameBanEdit->setMaxLength(MAX_NAME_LENGTH);
    ipBanCheckBox = new QCheckBox(tr("ban &IP address"));
    ipBanCheckBox->setChecked(true);
    ipBanEdit = new QLineEdit(QString::fromStdString(info.address()));
    ipBanEdit->setMaxLength(MAX_NAME_LENGTH);
    idBanCheckBox = new QCheckBox(tr("ban client I&D"));
    idBanCheckBox->setChecked(true);
    idBanEdit = new QLineEdit(QString::fromStdString(info.clientid()));
    idBanEdit->setMaxLength(MAX_NAME_LENGTH);
    if (QString::fromStdString(info.clientid()).isEmpty())
        idBanCheckBox->setChecked(false);

    QGridLayout *banTypeGrid = new QGridLayout;
    banTypeGrid->addWidget(nameBanCheckBox, 0, 0);
    banTypeGrid->addWidget(nameBanEdit, 0, 1);
    banTypeGrid->addWidget(ipBanCheckBox, 1, 0);
    banTypeGrid->addWidget(ipBanEdit, 1, 1);
    banTypeGrid->addWidget(idBanCheckBox, 2, 0);
    banTypeGrid->addWidget(idBanEdit, 2, 1);
    QGroupBox *banTypeGroupBox = new QGroupBox(tr("Ban type"));
    banTypeGroupBox->setLayout(banTypeGrid);

    permanentRadio = new QRadioButton(tr("&permanent ban"));
    temporaryRadio = new QRadioButton(tr("&temporary ban"));
    temporaryRadio->setChecked(true);
    connect(temporaryRadio, &QRadioButton::toggled, this, &BanDialog::enableTemporaryEdits);
    daysLabel = new QLabel(tr("&Days:"));
    daysEdit = new QSpinBox;
    daysEdit->setMinimum(0);
    daysEdit->setValue(0);
    daysEdit->setMaximum(10000);
    daysLabel->setBuddy(daysEdit);
    hoursLabel = new QLabel(tr("&Hours:"));
    hoursEdit = new QSpinBox;
    hoursEdit->setMinimum(0);
    hoursEdit->setValue(0);
    hoursEdit->setMaximum(24);
    hoursLabel->setBuddy(hoursEdit);
    minutesLabel = new QLabel(tr("&Minutes:"));
    minutesEdit = new QSpinBox;
    minutesEdit->setMinimum(0);
    minutesEdit->setValue(5);
    minutesEdit->setMaximum(60);
    minutesLabel->setBuddy(minutesEdit);
    QGridLayout *durationLayout = new QGridLayout;
    durationLayout->addWidget(permanentRadio, 0, 0, 1, 6);
    durationLayout->addWidget(temporaryRadio, 1, 0, 1, 6);
    durationLayout->addWidget(daysLabel, 2, 0);
    durationLayout->addWidget(daysEdit, 2, 1);
    durationLayout->addWidget(hoursLabel, 2, 2);
    durationLayout->addWidget(hoursEdit, 2, 3);
    durationLayout->addWidget(minutesLabel, 2, 4);
    durationLayout->addWidget(minutesEdit, 2, 5);
    QGroupBox *durationGroupBox = new QGroupBox(tr("Duration of the ban"));
    durationGroupBox->setLayout(durationLayout);

    QLabel *reasonLabel = new QLabel(tr("Please enter the reason for the ban.\nThis is only saved for moderators and "
                                        "cannot be seen by the banned person."));
    reasonEdit = new QPlainTextEdit;

    QLabel *visibleReasonLabel =
        new QLabel(tr("Please enter the reason for the ban that will be visible to the banned person."));
    visibleReasonEdit = new QPlainTextEdit;

    deleteMessages = new QCheckBox(tr("Redact all messages from this user in all rooms"));

    QPushButton *okButton = new QPushButton(tr("&OK"));
    okButton->setAutoDefault(true);
    connect(okButton, &QPushButton::clicked, this, &BanDialog::okClicked);
    QPushButton *cancelButton = new QPushButton(tr("&Cancel"));
    connect(cancelButton, &QPushButton::clicked, this, &BanDialog::reject);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);

    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->addWidget(banTypeGroupBox);
    vbox->addWidget(durationGroupBox);
    vbox->addWidget(reasonLabel);
    vbox->addWidget(reasonEdit);
    vbox->addWidget(visibleReasonLabel);
    vbox->addWidget(visibleReasonEdit);
    vbox->addWidget(deleteMessages);
    vbox->addLayout(buttonLayout);

    setLayout(vbox);
    setWindowTitle(tr("Ban user from server"));
}

WarningDialog::WarningDialog(const QString userName, const QString clientID, QWidget *parent) : QDialog(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    descriptionLabel = new QLabel(tr("Which warning would you like to send?"));
    nameWarning = new QLineEdit(userName);
    nameWarning->setMaxLength(MAX_NAME_LENGTH);
    warnClientID = new QLineEdit(clientID);
    warnClientID->setMaxLength(MAX_NAME_LENGTH);
    warningOption = new QComboBox();
    warningOption->addItem("");

    deleteMessages = new QCheckBox(tr("Redact all messages from this user in all rooms"));

    QPushButton *okButton = new QPushButton(tr("&OK"));
    okButton->setAutoDefault(true);
    connect(okButton, &QPushButton::clicked, this, &WarningDialog::okClicked);
    QPushButton *cancelButton = new QPushButton(tr("&Cancel"));
    connect(cancelButton, &QPushButton::clicked, this, &WarningDialog::reject);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);

    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->addWidget(descriptionLabel);
    vbox->addWidget(nameWarning);
    vbox->addWidget(warningOption);
    vbox->addWidget(deleteMessages);
    vbox->addLayout(buttonLayout);
    setLayout(vbox);
    setWindowTitle(tr("Warn user for misconduct"));
}

void WarningDialog::okClicked()
{
    if (nameWarning->text().simplified().isEmpty()) {
        QMessageBox::critical(this, tr("Error"),
                              tr("User name to send a warning to can not be blank, please specify a user to warn."));
        return;
    }

    if (warningOption->currentText().simplified().isEmpty()) {
        QMessageBox::critical(this, tr("Error"),
                              tr("Warning to use can not be blank, please select a valid warning to send."));
        return;
    }

    accept();
}

QString WarningDialog::getName() const
{
    return nameWarning->text().simplified();
}

QString WarningDialog::getWarnID() const
{
    return warnClientID->text().simplified();
}

QString WarningDialog::getReason() const
{
    return warningOption->currentText().simplified();
}

int WarningDialog::getDeleteMessages() const
{
    return deleteMessages->isChecked() ? -1 : 0;
}

void WarningDialog::addWarningOption(const QString warning)
{
    warningOption->addItem(warning);
}

void BanDialog::okClicked()
{
    if (!nameBanCheckBox->isChecked() && !ipBanCheckBox->isChecked() && !idBanCheckBox->isChecked()) {
        QMessageBox::critical(this, tr("Error"),
                              tr("You have to select a name-based, IP-based, clientId based, or some combination of "
                                 "the three to place a ban."));
        return;
    }

    if (nameBanCheckBox->isChecked())
        if (nameBanEdit->text().simplified() == "") {
            QMessageBox::critical(this, tr("Error"),
                                  tr("You must have a value in the name ban when selecting the name ban checkbox."));
            return;
        }

    if (ipBanCheckBox->isChecked())
        if (ipBanEdit->text().simplified() == "") {
            QMessageBox::critical(this, tr("Error"),
                                  tr("You must have a value in the ip ban when selecting the ip ban checkbox."));
            return;
        }

    if (idBanCheckBox->isChecked())
        if (idBanEdit->text().simplified() == "") {
            QMessageBox::critical(
                this, tr("Error"),
                tr("You must have a value in the clientid ban when selecting the clientid ban checkbox."));
            return;
        }

    accept();
}

void BanDialog::enableTemporaryEdits(bool enabled)
{
    daysLabel->setEnabled(enabled);
    daysEdit->setEnabled(enabled);
    hoursLabel->setEnabled(enabled);
    hoursEdit->setEnabled(enabled);
    minutesLabel->setEnabled(enabled);
    minutesEdit->setEnabled(enabled);
}

QString BanDialog::getBanId() const
{
    return idBanCheckBox->isChecked() ? idBanEdit->text() : QString();
}

QString BanDialog::getBanName() const
{
    return nameBanCheckBox->isChecked() ? nameBanEdit->text() : QString();
}

QString BanDialog::getBanIP() const
{
    return ipBanCheckBox->isChecked() ? ipBanEdit->text() : QString();
}

int BanDialog::getMinutes() const
{
    return permanentRadio->isChecked() ? 0
                                       : (daysEdit->value() * 24 * 60 + hoursEdit->value() * 60 + minutesEdit->value());
}

QString BanDialog::getReason() const
{
    return reasonEdit->toPlainText();
}

QString BanDialog::getVisibleReason() const
{
    return visibleReasonEdit->toPlainText();
}

int BanDialog::getDeleteMessages() const
{
    return deleteMessages->isChecked() ? -1 : 0;
}

AdminNotesDialog::AdminNotesDialog(const QString &_userName, const QString &_notes, QWidget *_parent)
    : QDialog(_parent), userName(_userName)
{
    setAttribute(Qt::WA_DeleteOnClose);

    auto *updateButton = new QPushButton(tr("Update Notes"));
    updateButton->setEnabled(false);
    connect(updateButton, &QPushButton::clicked, this, &AdminNotesDialog::accept);

    notes = new QPlainTextEdit(_notes);
    notes->setMinimumWidth(500);
    connect(notes, &QPlainTextEdit::textChanged, this, [=]() { updateButton->setEnabled(true); });

    auto *vbox = new QVBoxLayout;
    vbox->addWidget(notes);
    vbox->addWidget(updateButton);

    setLayout(vbox);
    setWindowTitle(tr("Admin Notes for %1").arg(_userName));
}

QString AdminNotesDialog::getNotes() const
{
    return notes->toPlainText();
}

UserListItemDelegate::UserListItemDelegate(QObject *const parent) : QStyledItemDelegate(parent)
{
}

bool UserListItemDelegate::editorEvent(QEvent *event,
                                       QAbstractItemModel *model,
                                       const QStyleOptionViewItem &option,
                                       const QModelIndex &index)
{
    if ((event->type() == QEvent::MouseButtonPress) && index.isValid()) {
        QMouseEvent *const mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent->button() == Qt::RightButton) {
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
            static_cast<UserListWidget *>(parent())->showContextMenu(mouseEvent->globalPosition().toPoint(), index);
#else
            static_cast<UserListWidget *>(parent())->showContextMenu(mouseEvent->globalPos(), index);
#endif
            return true;
        }
    }
    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

UserListTWI::UserListTWI(const ServerInfo_User &_userInfo) : QTreeWidgetItem(Type)
{
    setUserInfo(_userInfo);
}

void UserListTWI::setUserInfo(const ServerInfo_User &_userInfo)
{
    userInfo = _userInfo;

    setData(0, Qt::UserRole, userInfo.user_level());
    setIcon(0, UserLevelPixmapGenerator::generateIcon(18, UserLevelFlags(userInfo.user_level()), userInfo.pawn_colors(),
                                                      false, QString::fromStdString(userInfo.privlevel())));
    setIcon(1, QIcon(CountryPixmapGenerator::generatePixmap(18, QString::fromStdString(userInfo.country()))));
    setData(2, Qt::UserRole, QString::fromStdString(userInfo.name()));
    setData(2, Qt::DisplayRole, QString::fromStdString(userInfo.name()));
    setData(3, Qt::InitialSortOrderRole, QString::fromStdString(userInfo.privlevel()));
}

void UserListTWI::setOnline(bool online)
{
    setData(0, Qt::UserRole + 1, online);
    setData(2, Qt::ForegroundRole, online ? qApp->palette().brush(QPalette::WindowText) : QBrush(Qt::gray));
}

/**
 * Sort Users in the following order
 * 1) Online Users > Offline Users
 * 2) Admins, judge/vip/donator status ignored
 * 3) Moderators, judge/vip/donator status ignored
 * 4) Judges
 * 5) VIPs
 * 6) Donators
 * 7) Everyone else
 * @param other RHS to compare to
 * @return Left is less than the Right
 */
bool UserListTWI::operator<(const QTreeWidgetItem &other) const
{
    // Sort by online/offline
    if (data(0, Qt::UserRole + 1) != other.data(0, Qt::UserRole + 1)) {
        return data(0, Qt::UserRole + 1).toBool();
    }

    const auto &lhsUserLevelFlags = UserLevelFlags(data(0, Qt::UserRole).toInt());
    const auto &rhsUserLevelFlags = UserLevelFlags(other.data(0, Qt::UserRole).toInt());

    // Admins & Mods need no additional comparison checks, just to see if they're an admin or a moderator
    static const QList<ServerInfo_User_UserLevelFlag> userLevelWithNoOtherPrefOrder = {
        ServerInfo_User_UserLevelFlag_IsAdmin, ServerInfo_User_UserLevelFlag_IsModerator};
    for (const auto &userLevelEntry : userLevelWithNoOtherPrefOrder) {
        if (lhsUserLevelFlags.testFlag(userLevelEntry) &&
            lhsUserLevelFlags.testFlag(userLevelEntry) == rhsUserLevelFlags.testFlag(userLevelEntry)) {
            return QString::localeAwareCompare(data(2, Qt::UserRole).toString(),
                                               other.data(2, Qt::UserRole).toString()) < 0;
        } else if (lhsUserLevelFlags.testFlag(userLevelEntry) != rhsUserLevelFlags.testFlag(userLevelEntry)) {
            return lhsUserLevelFlags.testFlag(userLevelEntry) > rhsUserLevelFlags.testFlag(userLevelEntry);
        }
    }

    // Judges can be sorted by their additional ranks
    static const QList<ServerInfo_User_UserLevelFlag> userLevelOrder = {ServerInfo_User_UserLevelFlag_IsJudge,
                                                                        ServerInfo_User_UserLevelFlag_IsRegistered,
                                                                        ServerInfo_User_UserLevelFlag_IsUser};
    for (const auto &userLevelEntry : userLevelOrder) {
        if (lhsUserLevelFlags.testFlag(userLevelEntry) != rhsUserLevelFlags.testFlag(userLevelEntry)) {
            return lhsUserLevelFlags.testFlag(userLevelEntry) > rhsUserLevelFlags.testFlag(userLevelEntry);
        }
    }

    // Sort by VIP > Donator > None
    static const QMap<QString, int> privilegeOrder = {{"VIP", 3}, {"DONATOR", 2}, {"NONE", 1}, {"UNKNOWN", 0}};
    const auto &lhsUserPrivLevel = privilegeOrder.value(data(3, Qt::InitialSortOrderRole).toString(), 0);
    const auto &rhsUserPrivLevel = privilegeOrder.value(other.data(3, Qt::InitialSortOrderRole).toString(), 0);
    if (lhsUserPrivLevel != rhsUserPrivLevel) {
        return lhsUserPrivLevel > rhsUserPrivLevel;
    }

    // Sort by name
    return QString::localeAwareCompare(data(2, Qt::UserRole).toString(), other.data(2, Qt::UserRole).toString()) < 0;
}

UserListWidget::UserListWidget(TabSupervisor *_tabSupervisor,
                               AbstractClient *_client,
                               UserListType _type,
                               QWidget *parent)
    : QGroupBox(parent), tabSupervisor(_tabSupervisor), client(_client), type(_type), onlineCount(0)
{
    itemDelegate = new UserListItemDelegate(this);
    userContextMenu = new UserContextMenu(tabSupervisor, this);
    connect(userContextMenu, &UserContextMenu::openMessageDialog, this, &UserListWidget::openMessageDialog);

    userTree = new QTreeWidget;
    userTree->setColumnCount(3);
    userTree->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    userTree->header()->setMinimumSectionSize(0);
    userTree->setHeaderHidden(true);
    userTree->setRootIsDecorated(false);
    userTree->setIconSize(QSize(20, 18));
    userTree->setItemDelegate(itemDelegate);
    userTree->setAlternatingRowColors(true);
    connect(userTree, &QTreeWidget::itemActivated, this, &UserListWidget::userClicked);

    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->addWidget(userTree);

    setLayout(vbox);

    retranslateUi();
}

void UserListWidget::retranslateUi()
{
    userContextMenu->retranslateUi();
    switch (type) {
        case AllUsersList:
            titleStr = tr("Users connected to server: %1");
            break;
        case RoomList:
            titleStr = tr("Users in this room: %1");
            break;
        case BuddyList:
            titleStr = tr("Buddies online: %1 / %2");
            break;
        case IgnoreList:
            titleStr = tr("Ignored users online: %1 / %2");
            break;
    }
    updateCount();
}

void UserListWidget::processUserInfo(const ServerInfo_User &user, bool online)
{
    const QString userName = QString::fromStdString(user.name());
    UserListTWI *item = users.value(userName);
    if (item)
        item->setUserInfo(user);
    else {
        item = new UserListTWI(user);
        users.insert(userName, item);
        userTree->addTopLevelItem(item);
        if (online)
            ++onlineCount;
        updateCount();
    }
    item->setOnline(online);
}

bool UserListWidget::deleteUser(const QString &userName)
{
    UserListTWI *twi = users.value(userName);
    if (twi) {
        users.remove(userName);
        userTree->takeTopLevelItem(userTree->indexOfTopLevelItem(twi));
        if (twi->data(0, Qt::UserRole + 1).toBool())
            --onlineCount;
        delete twi;
        updateCount();
        return true;
    }

    return false;
}

void UserListWidget::setUserOnline(const QString &userName, bool online)
{
    UserListTWI *twi = users.value(userName);
    if (!twi)
        return;

    twi->setOnline(online);
    if (online)
        ++onlineCount;
    else
        --onlineCount;
    updateCount();
}

void UserListWidget::updateCount()
{
    QString str = titleStr;
    if ((type == BuddyList) || (type == IgnoreList))
        str = str.arg(onlineCount);
    setTitle(str.arg(userTree->topLevelItemCount()));
}

void UserListWidget::userClicked(QTreeWidgetItem *item, int /*column*/)
{
    emit openMessageDialog(item->data(2, Qt::UserRole).toString(), true);
}

void UserListWidget::showContextMenu(const QPoint &pos, const QModelIndex &index)
{
    const ServerInfo_User &userInfo = static_cast<UserListTWI *>(userTree->topLevelItem(index.row()))->getUserInfo();
    bool online = index.sibling(index.row(), 0).data(Qt::UserRole + 1).toBool();

    userContextMenu->showContextMenu(pos, QString::fromStdString(userInfo.name()),
                                     UserLevelFlags(userInfo.user_level()), online);
}

void UserListWidget::sortItems()
{
    userTree->sortItems(1, Qt::AscendingOrder);
}
