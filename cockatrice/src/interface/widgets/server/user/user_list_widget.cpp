#include "user_list_widget.h"

#include "../../../../client/settings/cache_settings.h"
#include "../../../card_picture_loader/card_picture_loader.h"
#include "../../interface/pixel_map_generator.h"
#include "../../interface/theme_manager.h"
#include "../../interface/widgets/tabs/tab_account.h"
#include "../../interface/widgets/tabs/tab_supervisor.h"
#include "../game_selector.h"
#include "user_context_menu.h"
#include "user_list_painter.h"

#include <QApplication>
#include <QCheckBox>
#include <QCursor>
#include <QFont>
#include <QFontMetrics>
#include <QFrame>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QScreen>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QWidget>
#include <libcockatrice/card/database/card_database_manager.h>
#include <libcockatrice/network/client/abstract/abstract_client.h>
#include <libcockatrice/protocol/pb/response_get_games_of_user.pb.h>
#include <libcockatrice/protocol/pb/response_get_user_info.pb.h>
#include <libcockatrice/protocol/pending_command.h>
#include <libcockatrice/settings/appearance_settings.h>
#include <libcockatrice/utility/string_limits.h>

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

// Divider items (section headers) in sectioned mode are distinguished from user
// rows (UserListTWI, which uses QTreeWidgetItem::Type) by this item type.
constexpr int SectionItemType = QTreeWidgetItem::UserType + 1;

UserListItemDelegate::UserListItemDelegate(UserListWidget *owner,
                                           QTreeWidget *tree,
                                           const QMap<QString, QPixmap> *avatarCache,
                                           const QMap<QString, QPixmap> *cardArtCache,
                                           const QMap<QString, CardArtParams> *cardArtParamsMap)
    : QStyledItemDelegate(tree), tree(tree), owner(owner), avatarCache(avatarCache), cardArtCache(cardArtCache),
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
            owner->showContextMenu(mouseEvent->globalPosition().toPoint(), index);
            return true;
        }
    }
    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

QSize UserListItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!SettingsCache::instance().appearance().getStyleUserList()) {
        return QStyledItemDelegate::sizeHint(option, index);
    }
    if (!index.data(UserListRoles::UserInfo).isValid()) {
        return QStyledItemDelegate::sizeHint(option, index); // section dividers stay compact
    }
    return UserListPainter::sizeHint();
}

void UserListItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    const bool styled = SettingsCache::instance().appearance().getStyleUserList();
    // UserInfo/Online are stored on column 0 only. The name lives on column 2,
    // so resolve the user data against column 0 no matter which cell is painted.
    const QModelIndex userIndex = index.siblingAtColumn(0).isValid() ? index.siblingAtColumn(0) : index;
    const QVariant var = userIndex.data(UserListRoles::UserInfo);

    if (styled && var.isValid()) {
        // Styled card rows: the tree's cached palette can be stale after a
        // runtime theme change, so paint from the application palette (always
        // current) instead of option.palette (frozen at the last style switch).
        QStyleOptionViewItem opt = option;
        opt.palette = qApp->palette();

        UserListPainter::paint(painter, opt, index, var.value<ServerInfo_User>(), avatarCache, cardArtCache,
                               cardArtParamsMap, themeManager && themeManager->isDarkModeActive());
        return;
    }

    // Unstyled rows and section dividers are painted manually so every color
    // is derived from the current application palette at paint time. The widget
    // palette, the view's alternation, and the stored brushes (one per item)
    // all go stale after a runtime theme change.
    const QPalette appPal = qApp->palette();
    const bool selected = option.state & QStyle::State_Selected;
    const bool hovered = option.state & QStyle::State_MouseOver;

    // Row background: selection, hover, zebra striping, plain base.
    QColor bg = appPal.color(QPalette::Base);
    if (selected) {
        bg = appPal.color(QPalette::Highlight);
    } else if (hovered) {
        bg = UserListPainter::blend(appPal.color(QPalette::Base), appPal.color(QPalette::Highlight), 0.12);
    } else if (var.isValid()) {
        // Zebra alternation per section. The dividers are top level rows too,
        // so the view's own alternation would drift across sections. Count the
        // visible user rows back to the previous divider within the same parent
        // (sectioned mode stores users as children of the dividers, flat mode
        // as top level rows), so the stripe restarts at every divider and at
        // the tree top. Hidden filter matches are skipped the same way the view
        // skips them, so adjacent visible rows always alternate.
        int usersSinceDivider = 0;
        const QModelIndex parent = userIndex.parent();
        for (int r = userIndex.row() - 1; r >= 0; --r) {
            if (tree->isRowHidden(r, parent)) {
                continue;
            }
            const QModelIndex above = userIndex.model()->index(r, 0, parent);
            if (above.isValid() && above.data(UserListRoles::UserInfo).isValid()) {
                ++usersSinceDivider;
            } else {
                break;
            }
        }
        if (usersSinceDivider % 2 == 1) {
            bg = appPal.color(QPalette::AlternateBase);
        }
    }
    // Paint the row background. In the column 0 pass the fill spans the full
    // viewport width so stripes, hover and selection cover the whole row (the
    // name column is content sized in unstyled mode). Later column passes fill
    // only their own cell, which is the same color and cannot cover the icons.
    QRect bgRect = option.rect;
    if (index.column() == 0) {
        bgRect = QRect(0, option.rect.top(), tree->viewport()->width(), option.rect.height());
    }
    painter->fillRect(bgRect, bg);

    // Text color.
    QColor fg = appPal.color(QPalette::Text);
    if (selected) {
        fg = appPal.color(QPalette::HighlightedText);
    } else if (!var.isValid()) {
        // Section divider: muted application text color.
        fg = appPal.color(QPalette::WindowText);
        fg.setAlpha(170);
    } else if (index.column() == 2) {
        // Name column: online/offline color recomputed at paint time instead
        // of trusting the brush stored at login time.
        QTreeWidgetItem *item = tree->itemFromIndex(index);
        const bool online = item && item->data(0, UserListRoles::Online).toBool();
        if (online) {
            fg = appPal.color(QPalette::WindowText);
        } else {
            fg = (themeManager && themeManager->isDarkModeActive())
                     ? QColor(Qt::gray)
                     : UserListPainter::blend(appPal.color(QPalette::Text), appPal.color(QPalette::Mid), 0.5);
        }
    }

    // Icon (level badge in column 0, country flag in column 1).
    QRect textRect = option.rect;
    const QIcon icon = index.data(Qt::DecorationRole).value<QIcon>();
    if (!icon.isNull()) {
        const QSize iconSize = icon.actualSize(QSize(18, 18));
        const QRect iconRect(option.rect.left() + 2, option.rect.center().y() - iconSize.height() / 2, iconSize.width(),
                             iconSize.height());
        icon.paint(painter, iconRect);
        textRect.setLeft(iconRect.right() + 4);
    }

    // Text (name column / divider title), elided to the row width.
    painter->save();
    painter->setPen(fg);
    const QFont itemFont = index.data(Qt::FontRole).value<QFont>();
    painter->setFont(itemFont.isCopyOf(QFont()) ? option.font : itemFont);
    const QString text = index.data(Qt::DisplayRole).toString();
    const QString elided = painter->fontMetrics().elidedText(text, Qt::ElideRight, textRect.width() - 4);
    painter->drawText(textRect.adjusted(2, 0, -2, 0), Qt::AlignLeft | Qt::AlignVCenter, elided);
    painter->restore();

    // Focus indicator for the current item.
    if (option.state & QStyle::State_HasFocus) {
        painter->setPen(appPal.color(QPalette::Highlight));
        painter->drawRect(option.rect.adjusted(0, 0, -1, -1));
    }
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
    // Only the online state is stored here: the delegate derives the
    // online/offline text color at paint time from the current application
    // palette, so no brush is cached (it would go stale on theme change).
    setData(0, UserListRoles::Online, online);
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

    userContextMenu = new UserContextMenu(tabSupervisor, this);
    connect(userContextMenu, &UserContextMenu::openMessageDialog, this, &UserListWidget::openMessageDialog);

    userTree = new QTreeWidget;
    userTree->setColumnCount(4); // 0=display, 1=flag(hidden), 2=name(hidden), 3=privlevel(hidden)
    userTree->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    userTree->header()->setMinimumSectionSize(0);
    userTree->setHeaderHidden(true);
    userTree->setRootIsDecorated(false);
    userTree->setIconSize(QSize(20, 18));
    itemDelegate = new UserListItemDelegate(this, userTree, &avatarProvider->cache(), &cardArtProvider->cache(),
                                            &cardArtParamsMap);
    userTree->setItemDelegate(itemDelegate);
    userTree->setAlternatingRowColors(true);
    userTree->hideColumn(1);
    userTree->hideColumn(2);
    userTree->hideColumn(3);
    connect(userTree, &QTreeWidget::itemActivated, this, &UserListWidget::userClicked);
    userTree->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    userTree->header()->setStretchLastSection(true);

    // ── Hover popup ───────────────────────────────────────────────────────────
    userInfoPopup = new UserInfoPopup(tabSupervisor, tabSupervisor->getClient(), &avatarProvider->cache(),
                                      &cardArtProvider->cache(), &cardArtParamsMap,
                                      window()); // parented to main window so it floats above siblings

    userInfoPopup->hide();
    userInfoPopup->setWindowOpacity(0.0);
    userInfoPopup->installEventFilter(this);

    showPopupTimer = new QTimer(this);
    showPopupTimer->setSingleShot(true);
    showPopupTimer->setInterval(280);
    connect(showPopupTimer, &QTimer::timeout, this, [this] {
        if (hoveredUser.isEmpty()) {
            return;
        }
        // Resolve the row under the cursor again. In sectioned mode a user can
        // own several rows (online + buddy), so the popup must anchor to the
        // exact hovered row instead of a lookup by name.
        const QPoint viewportPos = userTree->viewport()->mapFromGlobal(QCursor::pos());
        QTreeWidgetItem *item = userTree->itemAt(viewportPos);
        if (item && item->type() == QTreeWidgetItem::Type &&
            QString::fromStdString(static_cast<UserListTWI *>(item)->getUserInfo().name()) == hoveredUser) {
            showPopupForUser(static_cast<UserListTWI *>(item));
        }
    });

    hidePopupTimer = new QTimer(this);
    hidePopupTimer->setSingleShot(true);
    hidePopupTimer->setInterval(160);
    connect(hidePopupTimer, &QTimer::timeout, this, [this] {
        // The hover ends when the cursor leaves the user row. Empty list
        // space, a section divider and anything outside the tree all close
        // the popup, while the popup itself keeps it alive.
        if (!popupPinned && !userInfoPopup->underMouse() && (hoveredUser.isEmpty() || !userTree->underMouse())) {
            hidePopup();
        }
    });

    connectPopupSignals();

    userTree->setMouseTracking(true);
    userTree->viewport()->setMouseTracking(true);
    userTree->viewport()->installEventFilter(this);
    userTree->installEventFilter(this); // keyboard handling for section dividers

    // Clicking anywhere outside the list clears its selection and closes the
    // popup. The filter watches all widgets because the press can land on any
    // part of the window, on another list or on the popup itself.
    qApp->installEventFilter(this);

    // Pin on item click
    connect(userTree, &QTreeWidget::itemClicked, this, [this](QTreeWidgetItem *item, int) {
        // Clicking a section divider toggles it
        if (sectioned && item->type() == SectionItemType) {
            setExpandedProgrammatically(item, !item->isExpanded());
            handleSectionExpansion(item, item->isExpanded());
            return;
        }
        if (!SettingsCache::instance().appearance().getStyleUserList()) {
            return;
        }
        if (item->type() != QTreeWidgetItem::Type) {
            return; // divider rows have no user popup
        }
        popupPinned = false; // reset so showPopupForUser can update
        showPopupForUser(static_cast<UserListTWI *>(item));
        popupPinned = true; // pin after showing
    });

    connect(userTree->selectionModel(), &QItemSelectionModel::selectionChanged, this,
            [this](const QItemSelection &sel, const QItemSelection &) {
                if (sel.isEmpty() && popupPinned) {
                    popupPinned = false;
                    hidePopup();
                }
            });

    // Keyboard selection: show the popup for the current row and hide it when
    // the focus moves to a section divider or leaves the list entirely. The
    // popup therefore follows arrow key navigation exactly like mouse hover.
    // When it was pinned by a click it stays open and follows the selection.
    connect(userTree, &QTreeWidget::currentItemChanged, this, [this](QTreeWidgetItem *current, QTreeWidgetItem *) {
        if (!isVisible() || !SettingsCache::instance().appearance().getStyleUserList()) {
            return;
        }
        if (current && current->type() == QTreeWidgetItem::Type) {
            showPopupForUser(static_cast<UserListTWI *>(current));
        } else {
            popupPinned = false;
            hidePopup();
        }
    });

    // Section dividers can be collapsed/expanded by the user. Surface those
    // changes only from real user interaction. Programmatic expansion is
    // applied through setSectionExpanded() / setExpandedProgrammatically().
    connect(userTree, &QTreeWidget::itemExpanded, this,
            [this](QTreeWidgetItem *item) { handleSectionExpansion(item, true); });
    connect(userTree, &QTreeWidget::itemCollapsed, this,
            [this](QTreeWidgetItem *item) { handleSectionExpansion(item, false); });

    // Hide popup when list scrolls (reference row has moved)
    connect(userTree->verticalScrollBar(), &QScrollBar::valueChanged, this, [this] {
        showPopupTimer->stop();
        hidePopup(true);
        requestAvatarsForVisibleItems();
    });

    // Forward join requests from popup upward
    connect(userInfoPopup, &UserInfoPopup::joinGameRequested, this, &UserListWidget::joinGameRequested);

    connect(avatarProvider, &UserAvatarProvider::avatarUpdated, this, &UserListWidget::refreshVisibleUserHeader);
    connect(cardArtProvider, &UserCardArtProvider::cardArtUpdated, this, &UserListWidget::refreshVisibleUserHeader);

    connect(&SettingsCache::instance().appearance(), &AppearanceSettings::styleUserListChanged, this,
            &UserListWidget::applyDisplayMode);
    applyDisplayMode();

    // The tree's cached palette can go stale after a runtime theme change (Qt
    // freezes widget palettes when the style is switched), so rows and dividers
    // derive all colors from the application palette at paint time. The theme
    // change only needs a repaint to pick them up.
    if (themeManager) {
        connect(themeManager, &ThemeManager::themeChanged, this, [this] {
            userTree->viewport()->update();
            userTree->update();
        });
    }

    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->addWidget(userTree);

    setLayout(vbox);

    retranslateUi();
}

UserListWidget::~UserListWidget()
{
    qApp->removeEventFilter(this);
}

void UserListWidget::bind(UserListManager *mgr)
{
    manager = mgr;

    // ── Full rebuild: disconnect / reconnect / bulk initial load ──────────────
    connect(manager, &UserListManager::listReset, this, &UserListWidget::rebuild);

    if (!sectioned) {
        // Online users list (AllUsersList / RoomList)
        if (type == AllUsersList || type == RoomList) {
            connect(manager, &UserListManager::userJoinedOnline, this,
                    [this](const ServerInfo_User &user) { processUserInfo(user, true); });
            connect(manager, &UserListManager::userLeftOnline, this, [this](const QString &name) { deleteUser(name); });
        }

        // Buddy list
        if (type == BuddyList) {
            connect(manager, &UserListManager::addedToBuddyList, this, [this](const ServerInfo_User &user) {
                const QString name = QString::fromStdString(user.name());
                processUserInfo(user, manager->getOnlineUser(name) != nullptr);
            });
            connect(manager, &UserListManager::removedFromBuddyList, this,
                    [this](const QString &name) { deleteUser(name); });
            // Track online presence changes for buddies already in the tree
            connect(manager, &UserListManager::userJoinedOnline, this, [this](const ServerInfo_User &user) {
                const QString name = QString::fromStdString(user.name());
                if (users.contains(name)) {
                    users[name]->setUserInfo(user);
                    setUserOnline(name, true);
                }
            });
            connect(manager, &UserListManager::userLeftOnline, this, [this](const QString &name) {
                if (users.contains(name)) {
                    setUserOnline(name, false);
                }
            });
        }

        // Ignore list
        if (type == IgnoreList) {
            connect(manager, &UserListManager::addedToIgnoreList, this, [this](const ServerInfo_User &user) {
                const QString name = QString::fromStdString(user.name());
                processUserInfo(user, manager->getOnlineUser(name) != nullptr);
            });
            connect(manager, &UserListManager::removedFromIgnoreList, this,
                    [this](const QString &name) { deleteUser(name); });
        }
    } else {
        // Sectioned mode: one tree, every source feeds its own section.
        // Sections are pure membership views: the "Online" section holds every
        // currently online user, the "Buddy"/"Ignore" sections hold those
        // lists. A user can therefore appear in several sections at once (an
        // online buddy gets one row in each).
        connect(manager, &UserListManager::userJoinedOnline, this,
                [this](const ServerInfo_User &user) { handleOnlineChange(user); });
        connect(manager, &UserListManager::userLeftOnline, this,
                [this](const QString &name) { handleOnlineChangeLeft(name); });
        connect(manager, &UserListManager::addedToBuddyList, this,
                [this](const ServerInfo_User &user) { handleListAdd(Section::Buddy, user); });
        connect(manager, &UserListManager::removedFromBuddyList, this,
                [this](const QString &name) { handleListRemove(Section::Buddy, name); });
        connect(manager, &UserListManager::addedToIgnoreList, this,
                [this](const ServerInfo_User &user) { handleListAdd(Section::Ignore, user); });
        connect(manager, &UserListManager::removedFromIgnoreList, this,
                [this](const QString &name) { handleListRemove(Section::Ignore, name); });
    }

    // ── Popup button refresh ──────────────────────────────────────────────────
    // Any buddy/ignore mutation while the popup is open refreshes its buttons
    auto refreshIfPopupOpen = [this](const QString &name) {
        if (userInfoPopup && userInfoPopup->isVisible() && userInfoPopup->getCurrentUser() == name) {
            refreshPopupButtons(name);
        }
    };
    auto refreshCurrentPopup = [refreshIfPopupOpen](const ServerInfo_User &u) {
        refreshIfPopupOpen(QString::fromStdString(u.name()));
    };

    connect(manager, &UserListManager::addedToBuddyList, this, refreshCurrentPopup);
    connect(manager, &UserListManager::removedFromBuddyList, this, refreshIfPopupOpen);
    connect(manager, &UserListManager::addedToIgnoreList, this, refreshCurrentPopup);
    connect(manager, &UserListManager::removedFromIgnoreList, this, refreshIfPopupOpen);
    connect(manager, &UserListManager::userJoinedOnline, this, refreshCurrentPopup);
    connect(manager, &UserListManager::userLeftOnline, this, refreshIfPopupOpen);

    rebuild();
}

void UserListWidget::refreshVisibleUserHeader(const QString &name)
{
    userTree->viewport()->update();
    if (userInfoPopup->isVisible() && userInfoPopup->getCurrentUser() == name) {
        userInfoPopup->refreshHeader();
    }
}

void UserListWidget::refreshPopupButtons(const QString &userName)
{
    UserListTWI *item = users.value(userName);
    if (!item) {
        return;
    }

    const UserListProxy *proxy = tabSupervisor->getUserListManager();
    const bool online = item->data(0, UserListRoles::Online).toBool();
    const bool isBuddy = proxy->isUserBuddy(userName);
    const bool isIgn = proxy->isUserIgnored(userName);

    userInfoPopup->updateActionButtons(item->getUserInfo(), online, isBuddy, isIgn);
    positionPopup(item); // height may have changed, reposition
}

void UserListWidget::hideEvent(QHideEvent *e)
{
    QGroupBox::hideEvent(e);
    showPopupTimer->stop();
    hidePopupTimer->stop();
    hidePopup(true);
}

void UserListWidget::showEvent(QShowEvent *e)
{
    QGroupBox::showEvent(e);
    requestAvatarsForVisibleItems();
}

void UserListWidget::applyDisplayMode()
{
    const bool styled = SettingsCache::instance().appearance().getStyleUserList();

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

void UserListWidget::connectPopupSignals()
{
    connect(userInfoPopup, &UserInfoPopup::closeRequested, this, [this] {
        popupPinned = false;
        hidePopup(true);
    });
    connect(userInfoPopup, &UserInfoPopup::mouseEnteredPopup, hidePopupTimer, &QTimer::stop);
    connect(userInfoPopup, &UserInfoPopup::mouseLeftPopup, this, [this] {
        if (!popupPinned) {
            hidePopupTimer->start();
        }
    });

    // Wire all action signals to UserContextMenu::exec*()
    connect(userInfoPopup, &UserInfoPopup::chatRequested, userContextMenu, &UserContextMenu::execChat);
    connect(userInfoPopup, &UserInfoPopup::detailsRequested, userContextMenu, &UserContextMenu::execDetails);
    connect(userInfoPopup, &UserInfoPopup::showGamesRequested, userContextMenu, &UserContextMenu::execShowGames);
    connect(userInfoPopup, &UserInfoPopup::addBuddyRequested, userContextMenu, &UserContextMenu::execAddToBuddy);
    connect(userInfoPopup, &UserInfoPopup::removeBuddyRequested, userContextMenu,
            &UserContextMenu::execRemoveFromBuddy);
    connect(userInfoPopup, &UserInfoPopup::addIgnoreRequested, userContextMenu, &UserContextMenu::execAddToIgnore);
    connect(userInfoPopup, &UserInfoPopup::removeIgnoreRequested, userContextMenu,
            &UserContextMenu::execRemoveFromIgnore);
    connect(userInfoPopup, &UserInfoPopup::banRequested, userContextMenu, &UserContextMenu::execBan);
    connect(userInfoPopup, &UserInfoPopup::warnRequested, userContextMenu, &UserContextMenu::execWarn);
    connect(userInfoPopup, &UserInfoPopup::banHistoryRequested, userContextMenu, &UserContextMenu::execBanHistory);
    connect(userInfoPopup, &UserInfoPopup::warnHistoryRequested, userContextMenu, &UserContextMenu::execWarnHistory);
    connect(userInfoPopup, &UserInfoPopup::adminNotesRequested, userContextMenu, &UserContextMenu::execAdminNotes);
    connect(userInfoPopup, &UserInfoPopup::promoteToModRequested, this,
            [this](const QString &n) { userContextMenu->execAdjustMod(n, true); });
    connect(userInfoPopup, &UserInfoPopup::demoteFromModRequested, this,
            [this](const QString &n) { userContextMenu->execAdjustMod(n, false); });
    connect(userInfoPopup, &UserInfoPopup::promoteToJudgeRequested, this,
            [this](const QString &n) { userContextMenu->execAdjustJudge(n, true); });
    connect(userInfoPopup, &UserInfoPopup::demoteFromJudgeRequested, this,
            [this](const QString &n) { userContextMenu->execAdjustJudge(n, false); });
}

bool UserListWidget::eventFilter(QObject *obj, QEvent *event)
{
    // A press outside the tree, the popup and any open menu deselects the
    // list and closes the popup. The filter is installed application-wide, so
    // the target can be any widget in the window or another list.
    if (event->type() == QEvent::MouseButtonPress) {
        auto *pressTarget = qobject_cast<QWidget *>(obj);
        if (pressTarget && !isPressInsideListUi(pressTarget)) {
            clearSelectionAndClosePopup();
        }
    }

    // Keyboard navigation of the section dividers.
    // The dividers are selectable so arrow keys land on them. When one is the
    // current item, Enter/Space toggle it (like a button) and Left/Right follow
    // the tree convention (Left collapses, Right expands).
    if (obj == userTree && event->type() == QEvent::KeyPress) {
        auto *keyEvent = static_cast<QKeyEvent *>(event);
        QTreeWidgetItem *current = userTree->currentItem();
        if (sectioned && current && current->type() == SectionItemType) {
            const bool toggle = keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter ||
                                keyEvent->key() == Qt::Key_Space;
            const bool collapse = keyEvent->key() == Qt::Key_Left && current->isExpanded();
            const bool expand = keyEvent->key() == Qt::Key_Right && !current->isExpanded();
            if (toggle || collapse || expand) {
                const bool expanded = toggle ? !current->isExpanded() : expand;
                setExpandedProgrammatically(current, expanded);
                handleSectionExpansion(current, expanded);
                return true;
            }
        }
    }

    if (obj == userTree->viewport()) {
        if (event->type() == QEvent::MouseMove) {
            if (!SettingsCache::instance().appearance().getStyleUserList()) {
                return QGroupBox::eventFilter(obj, event);
            }
            auto *me = static_cast<QMouseEvent *>(event);
            QTreeWidgetItem *hoveredItem = userTree->itemAt(me->pos());
            QString hovName;
            if (hoveredItem && hoveredItem->type() == QTreeWidgetItem::Type) {
                hovName = QString::fromStdString(static_cast<UserListTWI *>(hoveredItem)->getUserInfo().name());
            }

            if (hovName != hoveredUser) {
                hoveredUser = hovName;
                if (!hovName.isEmpty()) {
                    hidePopupTimer->stop();
                    if (!popupPinned) {
                        showPopupTimer->start();
                    }
                } else {
                    showPopupTimer->stop();
                    if (!popupPinned) {
                        hidePopupTimer->start();
                    }
                }
            }
        } else if (event->type() == QEvent::Leave) {
            hoveredUser.clear();
            showPopupTimer->stop();
            if (!popupPinned) {
                hidePopupTimer->start();
            }
        }
    }

    return QGroupBox::eventFilter(obj, event);
}

void UserListWidget::showPopupForUser(UserListTWI *item)
{
    if (!item) {
        return;
    }

    const QString userName = QString::fromStdString(item->getUserInfo().name());
    avatarProvider->requestAvatar(userName); // ensure the hovered user's avatar is fetched promptly

    const ServerInfo_User &info = item->getUserInfo();
    const bool online = item->data(0, UserListRoles::Online).toBool();
    const bool isBuddy = userContextMenu->getUserListProxy()->isUserBuddy(userName);
    const bool isIgn = userContextMenu->getUserListProxy()->isUserIgnored(userName);

    // The popup is already showing this user (e.g. arrow key navigation between
    // the online/buddy rows of the same user): just reposition it.
    if (userInfoPopup->isVisible() && userInfoPopup->getCurrentUser() == userName) {
        positionPopup(item);
        return;
    }

    // Cancel any pending show/hide so a hover timer that armed before a row
    // selected via the keyboard cannot override it, and a pending hide cannot
    // kill the popup right after it appears.
    showPopupTimer->stop();
    hidePopupTimer->stop();

    userInfoPopup->showForUser(userName, info, online, isBuddy, isIgn);

    const bool wasVisible = userInfoPopup->isVisible();
    if (!wasVisible) {
        // Realize the native window at opacity 0 before positioning so that:
        //   1) move() applies to an existing native handle (not overridden by
        //      Qt's default centering logic on first show)
        //   2) adjustSize() inside positionPopup() can measure the final
        //      laid out geometry correctly
        userInfoPopup->setWindowOpacity(0.0);
    }
    userInfoPopup->show();
    userInfoPopup->raise();

    positionPopup(item); // geometry is accurate after show, so move() is not overridden

    if (wasVisible) {
        // Content swap while already open (hover or arrow key navigation):
        // keep the popup opaque instead of flashing through a fade on every
        // step.
        userInfoPopup->setWindowOpacity(1.0);
        return;
    }

    auto *fade = new QPropertyAnimation(userInfoPopup, "windowOpacity", userInfoPopup);
    fade->setDuration(120);
    fade->setStartValue(0.0);
    fade->setEndValue(1.0);
    fade->start(QAbstractAnimation::DeleteWhenStopped);
}

void UserListWidget::positionPopup(UserListTWI *item)
{
    if (!item) {
        return;
    }

    QWidget *vp = userTree->viewport();
    const QRect itemR = userTree->visualItemRect(item);
    const QPoint itemTL = vp->mapToGlobal(itemR.topLeft());
    const QPoint vpTL = vp->mapToGlobal(vp->rect().topLeft());
    const QPoint vpTR = vp->mapToGlobal(vp->rect().topRight());

    userInfoPopup->adjustSize();
    const int popW = userInfoPopup->width();
    const int popH = userInfoPopup->height();
    const int margin = 12;

    QScreen *activeScreen = QGuiApplication::screenAt(itemTL);
    if (!activeScreen) {
        activeScreen = window()->screen();
    }
    const QRect screen =
        activeScreen ? activeScreen->availableGeometry() : QGuiApplication::primaryScreen()->availableGeometry();

    // ── X: prefer the side with more space ───────────────────────────────────
    const int spaceLeft = vpTL.x() - screen.left() - margin;
    const int spaceRight = screen.right() - vpTR.x() - margin;
    int x;
    if (spaceLeft >= spaceRight) {
        x = (spaceLeft >= popW) ? (vpTL.x() - margin - popW) : (vpTR.x() + margin);
    } else {
        x = (spaceRight >= popW) ? (vpTR.x() + margin) : (vpTL.x() - margin - popW);
    }
    x = qBound(screen.left() + margin, x, screen.right() - popW - margin);

    // ── Y: grow down if there's room, otherwise grow up ───────────────────────
    const int itemTopY = itemTL.y();
    const int spaceBelow = screen.bottom() - itemTopY - margin;
    const int spaceAbove = itemTopY - screen.top() - margin;

    int y;
    if (spaceBelow >= popH) {
        y = itemTopY; // top edges align, popup grows downward
    } else if (spaceAbove >= popH) {
        y = itemTopY - popH; // bottom of popup meets top of item, grows upward
    } else {
        // Neither side fits cleanly — pick the roomier side and let clamp handle the rest
        y = (spaceBelow >= spaceAbove) ? itemTopY : (itemTopY - popH);
    }
    y = qBound(screen.top() + margin, y, screen.bottom() - popH - margin);

    userInfoPopup->move(x, y);
}

void UserListWidget::hidePopup(bool immediate)
{
    showPopupTimer->stop();
    hidePopupTimer->stop();
    if (!userInfoPopup->isVisible()) {
        return;
    }

    if (immediate) {
        userInfoPopup->hide();
        return;
    }

    // Fade out
    auto *fade = new QPropertyAnimation(userInfoPopup, "windowOpacity", userInfoPopup);
    fade->setDuration(100);
    fade->setStartValue(userInfoPopup->windowOpacity());
    fade->setEndValue(0.0);
    connect(fade, &QPropertyAnimation::finished, userInfoPopup, &QWidget::hide);
    fade->start(QAbstractAnimation::DeleteWhenStopped);
}

bool UserListWidget::isPressInsideListUi(const QWidget *widget) const
{
    const QWidget *w = widget;
    while (w) {
        if (w == userTree || w == userInfoPopup || qobject_cast<const QMenu *>(w)) {
            return true;
        }
        w = w->parentWidget();
    }
    return false;
}

void UserListWidget::clearSelectionAndClosePopup()
{
    popupPinned = false;
    hidePopup(true);
    userTree->clearSelection();
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

void UserListWidget::beginBulkLoad()
{
    bulkLoading = true;
}

void UserListWidget::endBulkLoad()
{
    bulkLoading = false;
    sortItems();
    updateCount(); // divider counts were deferred during the bulk build
    requestAvatarsForVisibleItems();
    userTree->viewport()->update();
}

bool UserListWidget::isItemNearViewport(const UserListTWI *item) const
{
    // Prefetch a full viewport of rows above and below so scrolling never shows
    // an unloaded row.
    const QRect nearView =
        userTree->viewport()->rect().adjusted(0, -userTree->viewport()->height(), 0, userTree->viewport()->height());
    return userTree->visualItemRect(item).intersects(nearView);
}

void UserListWidget::requestAvatarsForVisibleItems()
{
    if (sectioned) {
        // Top level items are dividers, user rows hang below them.
        for (const Section section : sectionIds) {
            QTreeWidgetItem *divider = sectionItems.value(section);
            if (!divider) {
                continue;
            }
            for (int i = 0; i < divider->childCount(); ++i) {
                auto *twi = static_cast<UserListTWI *>(divider->child(i));
                if (isItemNearViewport(twi)) {
                    avatarProvider->requestAvatar(QString::fromStdString(twi->getUserInfo().name()));
                }
            }
        }
        return;
    }

    for (int i = 0; i < userTree->topLevelItemCount(); ++i) {
        auto *twi = static_cast<UserListTWI *>(userTree->topLevelItem(i));
        if (isItemNearViewport(twi)) {
            avatarProvider->requestAvatar(QString::fromStdString(twi->getUserInfo().name()));
        }
    }
}

void UserListWidget::rebuild()
{
    userTree->clear();
    users.clear();
    sectionUsers.clear();
    cardArtParamsMap.clear();
    onlineCount = 0;

    if (sectioned) {
        createSectionItems();
    }

    if (!manager) {
        return;
    }

    if (sectioned) {
        // Every source feeds its own section. Users that belong to several
        // sources (an online buddy) get one row per section because
        // ensureSectionMembership() creates the row when it is missing.
        beginBulkLoad();
        const auto &onlineUsers = manager->getAllUsersList();
        for (auto it = onlineUsers.cbegin(); it != onlineUsers.cend(); ++it) {
            processUserInfo(Section::Online, it.value(), true);
        }
        const auto &buddyUsers = manager->getBuddyList();
        for (auto it = buddyUsers.cbegin(); it != buddyUsers.cend(); ++it) {
            processUserInfo(Section::Buddy, it.value(), manager->getOnlineUser(it.key()) != nullptr);
        }
        const auto &ignoreUsers = manager->getIgnoreList();
        for (auto it = ignoreUsers.cbegin(); it != ignoreUsers.cend(); ++it) {
            processUserInfo(Section::Ignore, it.value(), manager->getOnlineUser(it.key()) != nullptr);
        }
        endBulkLoad();
        applyFilter();
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

    beginBulkLoad();
    for (auto it = source->cbegin(); it != source->cend(); ++it) {
        processUserInfo(it.value(), manager->getOnlineUser(it.key()) != nullptr);
    }
    endBulkLoad();
    applyFilter();
}

void UserListWidget::updateCardArtParams(const ServerInfo_User &user, const QString &userName)
{
    // Always update params from the latest ServerInfo_User, whether the
    // item is new or existing, so a live server push refreshes the rendering.
    if (user.has_card_art_params()) {
        const auto &cap = user.card_art_params();
        CardArtParams params;
        params.cardName = QString::fromStdString(cap.card_name());
        params.cardProviderId = QString::fromStdString(cap.card_provider_id());
        params.marginPctL = cap.margin_pct_l();
        params.marginPctR = cap.margin_pct_r();
        params.verticalOffset = cap.vertical_offset();
        params.zoom = cap.zoom();
        cardArtParamsMap.insert(userName, params);
        cardArtProvider->requestCardArt(userName, params.cardName, params.cardProviderId);
    } else {
        cardArtParamsMap.remove(userName); // clear stale params on removal
    }
}

void UserListWidget::processUserInfo(const ServerInfo_User &user, bool online)
{
    const QString userName = QString::fromStdString(user.name());

    updateCardArtParams(user, userName);

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
        if (!bulkLoading && isItemNearViewport(item)) {
            avatarProvider->requestAvatar(userName);
        }
    }
    item->setOnline(online);
    if (!bulkLoading) {
        sortItems();
        applyFilter();
        userTree->viewport()->update();
    }
}

void UserListWidget::processUserInfo(Section section, const ServerInfo_User &user, bool online)
{
    ensureSectionMembership(section, user, online);
    if (!bulkLoading) {
        sortItems();
        applyFilter();
        userTree->viewport()->update();
    }
}

bool UserListWidget::deleteUser(const QString &userName)
{
    if (sectioned) {
        // The user may own several rows (one per section). Drop them all.
        bool removed = false;
        const QList<Section> sections = sectionUsers.keys(); // snapshot: maps mutate
        for (const Section section : sections) {
            removed = dropSectionMembership(section, userName) || removed;
        }
        if (removed && !bulkLoading) {
            sortItems();
            applyFilter();
            userTree->viewport()->update();
        }
        return removed;
    }

    UserListTWI *twi = users.value(userName);
    if (!twi) {
        return false;
    }

    users.remove(userName);
    if (twi->parent()) {
        twi->parent()->removeChild(twi); // sectioned mode: rows hang off a divider
    } else {
        userTree->takeTopLevelItem(userTree->indexOfTopLevelItem(twi));
    }
    if (twi->data(0, UserListRoles::Online).toBool()) {
        --onlineCount;
    }
    delete twi;
    updateCount();
    applyFilter();
    return true;
}

void UserListWidget::setUserOnline(const QString &userName, bool online)
{
    if (sectioned) {
        // The rows in the "Online" section are created/removed by the presence
        // handlers. This only keeps the presence flag of the surviving rows
        // (e.g. a buddy row after the user went offline) in sync.
        for (auto it = sectionUsers.cbegin(); it != sectionUsers.cend(); ++it) {
            UserListTWI *item = it.value().value(userName);
            if (item) {
                item->setOnline(online);
            }
        }
        return;
    }

    UserListTWI *twi = users.value(userName);
    if (!twi) {
        return;
    }

    // No state change: nothing to resort. This also keeps the presence
    // broadcasts cheap (userJoinedOnline fires once per online user) when the
    // row already carries the right flag.
    if (twi->data(0, UserListRoles::Online).toBool() == online) {
        return;
    }

    twi->setOnline(online);
    if (online) {
        ++onlineCount;
    } else {
        --onlineCount;
    }
    updateCount();

    // Online users sort above offline users (UserListTWI::operator<), so a
    // flag change moves the row. Resort to place the user by the new state.
    if (!bulkLoading) {
        sortItems();
        applyFilter();
        userTree->viewport()->update();
    }
}

void UserListWidget::updateCount()
{
    if (sectioned) {
        // The dividers carry the section titles
        setTitle(QString());
        for (const Section section : sectionIds) {
            updateSectionDivider(section);
        }
        return;
    }

    if (showTitle) {
        QString str = titleStr;
        if ((type == BuddyList) || (type == IgnoreList)) {
            str = str.arg(onlineCount);
        }
        setTitle(str.arg(userTree->topLevelItemCount()));
    } else {
        setTitle(QString());
    }
}

void UserListWidget::setShowTitle(bool showTitle)
{
    this->showTitle = showTitle;
    updateCount();
}

void UserListWidget::setFilterText(const QString &text)
{
    if (filterText == text) {
        return;
    }
    filterText = text;
    applyFilter();
}

void UserListWidget::applyFilter()
{
    if (sectioned) {
        const bool searching = !filterText.isEmpty();
        const QString lower = filterText.toLower();
        for (const Section section : sectionIds) {
            QTreeWidgetItem *divider = sectionItems.value(section);
            if (!divider) {
                continue;
            }
            int visible = 0;
            for (int i = 0; i < divider->childCount(); ++i) {
                auto *child = static_cast<UserListTWI *>(divider->child(i));
                const bool match =
                    !searching || QString::fromStdString(child->getUserInfo().name()).toLower().contains(lower);
                child->setHidden(!match);
                if (match) {
                    ++visible;
                }
            }
            if (searching) {
                // During a search the sections with matches stay open and empty
                // sections disappear entirely. The persisted expansion state is
                // untouched and restored when the search is cleared.
                divider->setHidden(visible == 0);
                setExpandedProgrammatically(divider, visible > 0);
            } else {
                divider->setHidden(false);
                setExpandedProgrammatically(divider, expandedSections.contains(section));
            }
            updateSectionDivider(section);
        }
        requestAvatarsForVisibleItems();
        userTree->viewport()->update();
        return;
    }

    if (filterText.isEmpty()) {
        for (auto it = users.cbegin(); it != users.cend(); ++it) {
            it.value()->setHidden(false);
        }
    } else {
        const QString lower = filterText.toLower();
        for (auto it = users.cbegin(); it != users.cend(); ++it) {
            const bool match = QString::fromStdString(it.value()->getUserInfo().name()).toLower().contains(lower);
            it.value()->setHidden(!match);
        }
    }

    requestAvatarsForVisibleItems();
    userTree->viewport()->update();
}

void UserListWidget::userClicked(QTreeWidgetItem *item, int /*column*/)
{
    if (item->type() != QTreeWidgetItem::Type) {
        return; // divider rows open no chat
    }
    emit openMessageDialog(item->data(2, Qt::UserRole).toString(), true);
}

void UserListWidget::showContextMenu(const QPoint &pos, const QModelIndex &index)
{
    QTreeWidgetItem *item = userTree->itemFromIndex(index);
    if (!item || item->type() != QTreeWidgetItem::Type) {
        return; // divider rows have no user menu
    }
    const auto *userItem = static_cast<UserListTWI *>(item);
    const ServerInfo_User &userInfo = userItem->getUserInfo();
    const bool online = userItem->data(0, UserListRoles::Online).toBool();

    userContextMenu->showContextMenu(pos, QString::fromStdString(userInfo.name()),
                                     UserLevelFlags(userInfo.user_level()), online);
}

void UserListWidget::sortItems()
{
    if (sectioned) {
        // Sorting must stay inside each section so the dividers keep their
        // places as top level items.
        for (auto it = sectionItems.cbegin(); it != sectionItems.cend(); ++it) {
            it.value()->sortChildren(0, Qt::AscendingOrder);
        }
        return;
    }
    userTree->sortItems(0, Qt::AscendingOrder);
}

// Sectioned mode

void UserListWidget::setSectioned(const QList<Section> &ids)
{
    if (sectioned || ids.isEmpty()) {
        return;
    }

    sectioned = true;
    sectionIds = ids;
    expandedSections.clear();
    for (const Section section : sectionIds) {
        expandedSections.insert(section); // everything starts expanded
    }

    // The single tree owns scrolling and the dividers carry the section titles,
    // so the group box chrome and tree decorations collapse into a flat list.
    setFlat(true);
    setShowTitle(false);
    userTree->setFrameStyle(QFrame::NoFrame);
    // No tree branches: the dividers draw their own arrow glyph, so the rows can
    // sit flush with the left border.
    userTree->setRootIsDecorated(false);
    userTree->setIndentation(0);
    userTree->setAlternatingRowColors(false);
    if (auto *listLayout = layout()) {
        listLayout->setContentsMargins(0, 0, 0, 0);
    }

    createSectionItems();
    updateCount();
}

void UserListWidget::createSectionItems()
{
    sectionItems.clear();
    QSignalBlocker blocker(userTree); // no expansion signals while building
    for (const Section section : sectionIds) {
        QTreeWidgetItem *divider = createSectionItem(section);
        sectionItems.insert(section, divider);
        divider->setExpanded(expandedSections.contains(section));
    }
}

QTreeWidgetItem *UserListWidget::createSectionItem(Section section)
{
    Q_UNUSED(section);
    auto *divider = new QTreeWidgetItem(SectionItemType);
    // Selectable so keyboard navigation (Up/Down) can land on the dividers.
    // They act as collapsible section headers once they have focus.
    divider->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

    QFont font = userTree->font();
    font.setBold(true);
    divider->setFont(0, font);
    // A little taller than a plain text row so the header reads as a section
    // separator without matching the full user row height.
    divider->setSizeHint(0, QSize(0, QFontMetrics(font).height() + 16));

    userTree->addTopLevelItem(divider);
    // QTreeWidgetItem::setFirstColumnSpanned() does nothing while the item is
    // detached from the tree (Qt returns early when treeModel() is null), so it
    // must be called after addTopLevelItem(). Without the span the divider text
    // is confined to column 0 and gets elided in unstyled mode.
    divider->setFirstColumnSpanned(true);
    return divider;
}

QString UserListWidget::sectionTitle(Section section) const
{
    switch (section) {
        case Section::Buddy:
            return tr("Buddies");
        case Section::Online:
            return tr("Online");
        case Section::Ignore:
            return tr("Ignored");
    }
    return {};
}

void UserListWidget::updateSectionDivider(Section section)
{
    QTreeWidgetItem *divider = sectionItems.value(section);
    if (!divider) {
        return;
    }
    int visible = 0;
    int online = 0;
    for (int i = 0; i < divider->childCount(); ++i) {
        QTreeWidgetItem *child = divider->child(i);
        if (!child->isHidden()) {
            ++visible;
            if (child->data(0, UserListRoles::Online).toBool()) {
                ++online;
            }
        }
    }
    // The tree draws no branches (rows are flush), so the divider carries its
    // own collapse arrow glyph.
    const QString arrow = divider->isExpanded() ? QStringLiteral("\u25BE") : QStringLiteral("\u25B8");
    if (section == Section::Buddy) {
        // The buddy divider reports how many of the shown buddies are online,
        // mirroring the "Buddies online: %1 / %2" title of the non-sectioned
        // buddy list.
        divider->setText(0, tr("%1 %2 (%3/%4)").arg(arrow, sectionTitle(section)).arg(online).arg(visible));
    } else {
        divider->setText(0, tr("%1 %2 (%3)").arg(arrow, sectionTitle(section)).arg(visible));
    }
}

void UserListWidget::handleSectionExpansion(QTreeWidgetItem *item, bool expanded)
{
    if (!sectioned || item->type() != SectionItemType) {
        return;
    }
    // Reverse lookup. Only three dividers exist, so a linear scan over the
    // section map is cheaper than caching the section on each divider.
    auto dividerIt = sectionItems.constBegin();
    while (dividerIt != sectionItems.constEnd() && dividerIt.value() != item) {
        ++dividerIt;
    }
    if (dividerIt == sectionItems.constEnd()) {
        return;
    }
    const Section section = dividerIt.key();
    if (expanded) {
        expandedSections.insert(section);
    } else {
        expandedSections.remove(section);
    }
    updateSectionDivider(section); // the arrow glyph follows the state
    emit sectionExpanded(section, expanded);
}

void UserListWidget::setExpandedProgrammatically(QTreeWidgetItem *item, bool expanded)
{
    QSignalBlocker blocker(userTree);
    item->setExpanded(expanded);
}

void UserListWidget::setSectionExpanded(Section section, bool expanded)
{
    if (!sectioned) {
        return;
    }
    if (expanded) {
        expandedSections.insert(section);
    } else {
        expandedSections.remove(section);
    }
    QTreeWidgetItem *divider = sectionItems.value(section);
    if (!divider) {
        return;
    }
    QSignalBlocker blocker(userTree);
    divider->setExpanded(expanded);
    updateSectionDivider(section); // the arrow glyph follows the state
    userTree->viewport()->update();
}

void UserListWidget::handleOnlineChange(const ServerInfo_User &user)
{
    // A user came online: they get a row in the "Online" section, plus (if
    // applicable) a row in the buddy/ignore sections, which flip to online.
    const QString name = QString::fromStdString(user.name());
    ensureSectionMembership(Section::Online, user, true);
    if (manager->isUserBuddy(name)) {
        ensureSectionMembership(Section::Buddy, user, true);
    }
    if (manager->isUserIgnored(name)) {
        ensureSectionMembership(Section::Ignore, user, true);
    }
    finishSectionedMutation();
}

void UserListWidget::handleOnlineChangeLeft(const QString &userName)
{
    // The user is no longer online: their "Online" row disappears. Buddies and
    // ignored users keep their own section's row, marked offline. A plain user
    // has no rows left.
    const bool dropped = dropSectionMembership(Section::Online, userName);
    const bool kept = manager->isUserBuddy(userName) || manager->isUserIgnored(userName);
    if (kept) {
        setUserOnline(userName, false);
    }
    if (dropped || kept) {
        finishSectionedMutation();
    }
}

void UserListWidget::handleListAdd(Section section, const ServerInfo_User &user)
{
    const QString name = QString::fromStdString(user.name());
    const bool online = manager->getOnlineUser(name) != nullptr;
    ensureSectionMembership(section, user, online);
    if (online) {
        // The user belongs to the "Online" section as well. Make sure the row
        // exists even if the join event raced ahead of the list mutation.
        ensureSectionMembership(Section::Online, user, true);
    }
    finishSectionedMutation();
}

void UserListWidget::handleListRemove(Section section, const QString &userName)
{
    // Only the row of the removed section disappears: an online user keeps
    // their "Online" row, and other list memberships keep theirs.
    if (dropSectionMembership(section, userName)) {
        finishSectionedMutation();
    }
}

UserListTWI *UserListWidget::ensureSectionMembership(Section section, const ServerInfo_User &user, bool online)
{
    const QString userName = QString::fromStdString(user.name());

    updateCardArtParams(user, userName);

    QTreeWidgetItem *divider = sectionItems.value(section);
    if (!divider) {
        return nullptr;
    }

    QMap<QString, UserListTWI *> &sectionMap = sectionUsers[section];
    UserListTWI *item = sectionMap.value(userName);
    if (!item) {
        item = new UserListTWI(user);
        sectionMap.insert(userName, item);
        divider->addChild(item);
        if (!users.contains(userName)) {
            users.insert(userName, item); // primary row for lookups by name
        }
        // The divider counts are refreshed once in endBulkLoad(). Calling
        // updateCount() per row during a large rebuild would be quadratic.
        if (!bulkLoading) {
            updateCount(); // a new row changes the divider's count
        }
        if (!bulkLoading && isItemNearViewport(item)) {
            avatarProvider->requestAvatar(userName);
        }
    } else {
        item->setUserInfo(user);
    }
    item->setOnline(online);
    return item;
}

bool UserListWidget::dropSectionMembership(Section section, const QString &userName)
{
    QMap<QString, UserListTWI *> &sectionMap = sectionUsers[section];
    UserListTWI *item = sectionMap.take(userName);
    if (!item) {
        return false;
    }

    if (item->parent()) {
        item->parent()->removeChild(item);
    } else {
        userTree->takeTopLevelItem(userTree->indexOfTopLevelItem(item));
    }
    if (users.value(userName) == item) {
        // Repoint the primary row at another surviving row, if any.
        UserListTWI *replacement = nullptr;
        for (auto it = sectionUsers.cbegin(); it != sectionUsers.cend() && !replacement; ++it) {
            replacement = it.value().value(userName);
        }
        if (replacement) {
            users.insert(userName, replacement);
        } else {
            users.remove(userName);
        }
    }
    delete item;
    updateCount();
    return true;
}

void UserListWidget::finishSectionedMutation()
{
    if (bulkLoading) {
        return;
    }
    sortItems();
    applyFilter();
    userTree->viewport()->update();
}
