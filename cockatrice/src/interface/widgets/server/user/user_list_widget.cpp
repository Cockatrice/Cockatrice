#include "user_list_widget.h"

#include "../../../../client/settings/cache_settings.h"
#include "../../../card_picture_loader/card_picture_loader.h"
#include "../../interface/pixel_map_generator.h"
#include "../../interface/widgets/tabs/tab_account.h"
#include "../../interface/widgets/tabs/tab_supervisor.h"
#include "../game_selector.h"
#include "user_context_menu.h"
#include "user_list_painter.h"

#include <QApplication>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QWidget>
#include <libcockatrice/card/database/card_database_manager.h>
#include <libcockatrice/network/client/abstract/abstract_client.h>
#include <libcockatrice/protocol/pb/response_get_games_of_user.pb.h>
#include <libcockatrice/protocol/pb/response_get_user_info.pb.h>
#include <libcockatrice/protocol/pending_command.h>
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
    if (QString::fromStdString(info.clientid()).isEmpty()) {
        idBanCheckBox->setChecked(false);
    }

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

    if (nameBanCheckBox->isChecked()) {
        if (nameBanEdit->text().simplified() == "") {
            QMessageBox::critical(this, tr("Error"),
                                  tr("You must have a value in the name ban when selecting the name ban checkbox."));
            return;
        }
    }

    if (ipBanCheckBox->isChecked()) {
        if (ipBanEdit->text().simplified() == "") {
            QMessageBox::critical(this, tr("Error"),
                                  tr("You must have a value in the ip ban when selecting the ip ban checkbox."));
            return;
        }
    }

    if (idBanCheckBox->isChecked()) {
        if (idBanEdit->text().simplified() == "") {
            QMessageBox::critical(
                this, tr("Error"),
                tr("You must have a value in the clientid ban when selecting the clientid ban checkbox."));
            return;
        }
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

namespace UserListRoles
{
constexpr int Online = Qt::UserRole + 1;
constexpr int UserInfo = Qt::UserRole + 2;
} // namespace UserListRoles

UserListItemDelegate::UserListItemDelegate(QObject *const parent,
                                           const QMap<QString, QPixmap> *avatarCache,
                                           const QMap<QString, QPixmap> *cardArtCache,
                                           const QMap<QString, CardArtParams> *cardArtParamsMap)
    : QStyledItemDelegate(parent), avatarCache(avatarCache), cardArtCache(cardArtCache),
      cardArtParamsMap(cardArtParamsMap)
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

QSize UserListItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!SettingsCache::instance().getStyleUserList()) {
        return QStyledItemDelegate::sizeHint(option, index);
    }
    return UserListPainter::sizeHint();
}

void UserListItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!SettingsCache::instance().getStyleUserList()) {
        QStyledItemDelegate::paint(painter, option, index);
        return;
    }

    const QVariant var = index.data(UserListRoles::UserInfo);

    if (!var.isValid()) {
        QStyledItemDelegate::paint(painter, option, index);
        return;
    }

    UserListPainter::paint(painter, option, index, var.value<ServerInfo_User>(), avatarCache, cardArtCache,
                           cardArtParamsMap);
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
    setData(0, UserListRoles::UserInfo, QVariant::fromValue(userInfo));
}

void UserListTWI::setOnline(bool online)
{
    setData(0, UserListRoles::Online, online);
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
    if (data(0, UserListRoles::Online) != other.data(0, UserListRoles::Online)) {
        return data(0, UserListRoles::Online).toBool();
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
    avatarProvider = new UserAvatarProvider(client, this);
    cardArtProvider = new UserCardArtProvider(this);

    itemDelegate =
        new UserListItemDelegate(this, &avatarProvider->cache(), &cardArtProvider->cache(), &cardArtParamsMap);

    userContextMenu = new UserContextMenu(tabSupervisor, this);
    connect(userContextMenu, &UserContextMenu::openMessageDialog, this, &UserListWidget::openMessageDialog);

    userTree = new QTreeWidget;
    userTree->setColumnCount(4); // 0=display, 1=flag(hidden), 2=name(hidden), 3=privlevel(hidden)
    userTree->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    userTree->header()->setMinimumSectionSize(0);
    userTree->setHeaderHidden(true);
    userTree->setRootIsDecorated(false);
    userTree->setIconSize(QSize(20, 18));
    userTree->setItemDelegate(itemDelegate);
    userTree->setAlternatingRowColors(true);
    userTree->hideColumn(1);
    userTree->hideColumn(2);
    userTree->hideColumn(3);
    connect(userTree, &QTreeWidget::itemActivated, this, &UserListWidget::userClicked);
    userTree->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    userTree->header()->setStretchLastSection(true);

    connect(avatarProvider, &UserAvatarProvider::avatarUpdated, this,
            [this](const QString &) { userTree->viewport()->update(); });
    connect(cardArtProvider, &UserCardArtProvider::cardArtUpdated, this,
            [this](const QString &) { userTree->viewport()->update(); });

    connect(&SettingsCache::instance(), &SettingsCache::styleUserListChanged, this, &UserListWidget::applyDisplayMode);
    applyDisplayMode();

    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->addWidget(userTree);

    setLayout(vbox);

    retranslateUi();
}

void UserListWidget::bind(UserListManager *mgr)
{
    manager = mgr;

    connect(manager, &UserListManager::listsChanged, this, &UserListWidget::rebuild);

    rebuild();
}

void UserListWidget::applyDisplayMode()
{
    const bool styled = SettingsCache::instance().getStyleUserList();

    if (styled) {
        userTree->header()->setSectionResizeMode(0, QHeaderView::Stretch);
        userTree->hideColumn(1);
        userTree->hideColumn(2);
        userTree->hideColumn(3);
    } else {
        userTree->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
        userTree->showColumn(1);
        userTree->showColumn(2);
        userTree->hideColumn(3);
    }

    userTree->viewport()->update();
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

void UserListWidget::rebuild()
{
    userTree->clear();
    users.clear();
    cardArtParamsMap.clear();
    onlineCount = 0;

    if (!manager) {
        return;
    }

    const QMap<QString, ServerInfo_User> *source = nullptr;

    switch (type) {
        case AllUsersList:
        case RoomList:
            source = &manager->getAllUsersList();
            break;
        case BuddyList:
            source = &manager->getBuddyList();
            break;
        case IgnoreList:
            source = &manager->getIgnoreList();
            break;
    }

    for (auto it = source->cbegin(); it != source->cend(); ++it) {
        processUserInfo(it.value(), manager->getOnlineUser(it.key()) != nullptr);
    }

    sortItems();
}

void UserListWidget::processUserInfo(const ServerInfo_User &user, bool online)
{
    const QString userName = QString::fromStdString(user.name());

    // Always update params from the latest ServerInfo_User, whether the
    // item is new or existing, so a live server-push refreshes the rendering.
    if (user.has_card_art_params()) {
        const auto &cap = user.card_art_params();
        CardArtParams params;
        params.cardName = QString::fromStdString(cap.card_name());
        params.marginPctL = cap.margin_pct_l();
        params.marginPctR = cap.margin_pct_r();
        params.verticalOffset = cap.vertical_offset();
        params.zoom = cap.zoom();
        cardArtParamsMap.insert(userName, params);
        cardArtProvider->requestCardArt(userName, params.cardName);
    } else {
        cardArtParamsMap.remove(userName); // clear stale params on removal
    }

    UserListTWI *item = users.value(userName);
    if (item) {
        item->setUserInfo(user);
    } else {
        item = new UserListTWI(user);
        users.insert(userName, item);
        userTree->addTopLevelItem(item);
        if (online) {
            ++onlineCount;
        }
        updateCount();
        avatarProvider->requestAvatar(userName);
    }
    item->setOnline(online);
    userTree->viewport()->update();
}

bool UserListWidget::deleteUser(const QString &userName)
{
    UserListTWI *twi = users.value(userName);
    if (!twi) {
        return false;
    }

    users.remove(userName);
    userTree->takeTopLevelItem(userTree->indexOfTopLevelItem(twi));
    if (twi->data(0, Qt::UserRole + 1).toBool()) {
        --onlineCount;
    }
    delete twi;
    updateCount();
    return true;
}

void UserListWidget::setUserOnline(const QString &userName, bool online)
{
    UserListTWI *twi = users.value(userName);
    if (!twi) {
        return;
    }

    twi->setOnline(online);
    if (online) {
        ++onlineCount;
    } else {
        --onlineCount;
    }
    updateCount();
}

void UserListWidget::updateCount()
{
    QString str = titleStr;
    if ((type == BuddyList) || (type == IgnoreList)) {
        str = str.arg(onlineCount);
    }
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
    userTree->sortItems(0, Qt::AscendingOrder);
}
