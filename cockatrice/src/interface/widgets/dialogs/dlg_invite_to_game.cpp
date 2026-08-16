#include "dlg_invite_to_game.h"

#include "../server/user/user_list_manager.h"
#include "../server/user/user_list_widget.h"
#include "../tabs/tab_supervisor.h"

#include <QGuiApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScreen>
#include <QUrl>
#include <QUrlQuery>
#include <QVBoxLayout>

DlgInviteToGame::DlgInviteToGame(TabSupervisor *_tabSupervisor,
                                 const QString &_inviteUrl,
                                 bool _onlyBuddies,
                                 const QStringList &_excludeUserNames,
                                 QWidget *parent)
    : QDialog(parent), tabSupervisor(_tabSupervisor), inviteUrl(_inviteUrl), onlyBuddies(_onlyBuddies),
      excludeUserNames(_excludeUserNames)
{
    setModal(true);

    searchEdit = new QLineEdit(this);
    searchEdit->setClearButtonEnabled(true);
    connect(searchEdit, &QLineEdit::textChanged, this, &DlgInviteToGame::searchTextChanged);

    // The embedded list is the real room user list without the hover popup:
    // same manager, same delegate/painter, same sections, live via manager
    // signals while the modal loop runs.
    UserListManager *manager = tabSupervisor->getUserListManager();
    userList = new UserListWidget(tabSupervisor, tabSupervisor->getClient(), UserListWidget::RoomList, this,
                                  /*withUserInfoPopup=*/false);
    userList->setUserFilter([this, manager](const QString &name, bool online) {
        return !excludeUserNames.contains(name) && online && !manager->isUserIgnored(name);
    });
    if (onlyBuddies) {
        userList->setSectioned({UserListWidget::Section::Buddy});
    } else {
        userList->setSectioned({UserListWidget::Section::Buddy, UserListWidget::Section::Online});
    }
    userList->bind(manager);
    userList->rebuild();

    connect(userList, &UserListWidget::userActivated, this, &DlgInviteToGame::inviteCurrentUser);
    connect(userList, &UserListWidget::currentUserChanged, this, [this](const QString &userName) {
        currentUserName = userName;
        inviteButton->setEnabled(!userName.isEmpty());
    });
    connect(userList, &UserListWidget::userListChanged, this, &DlgInviteToGame::updateEmptyLabel);

    emptyLabel = new QLabel(this);
    emptyLabel->setAlignment(Qt::AlignCenter);
    emptyLabel->setVisible(false);

    inviteButton = new QPushButton(this);
    inviteButton->setEnabled(false);
    inviteButton->setDefault(true);
    connect(inviteButton, &QPushButton::clicked, this, [this] { inviteCurrentUser(currentUserName); });

    cancelButton = new QPushButton(this);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    auto *buttonRow = new QHBoxLayout;
    buttonRow->addStretch();
    buttonRow->addWidget(inviteButton);
    buttonRow->addWidget(cancelButton);

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(searchEdit);
    layout->addWidget(userList, 1);
    layout->addWidget(emptyLabel);
    layout->addLayout(buttonRow);

    updateEmptyLabel();
    retranslateUi();

    // Default to a comfortably tall dialog so the list has room to breathe,
    // capped by the available screen. No minimum is enforced: small screens
    // and manual resizing can go shorter than this.
    const QRect availableScreen = QGuiApplication::primaryScreen()->availableGeometry();
    resize(sizeHint().width(), qMin(sizeHint().height() * 3, availableScreen.height() * 4 / 5));
}

void DlgInviteToGame::searchTextChanged(const QString &text)
{
    userList->setFilterText(text);
    updateEmptyLabel();
}

void DlgInviteToGame::updateEmptyLabel()
{
    emptyLabel->setVisible(userList->visibleUserRowCount() == 0);
}

void DlgInviteToGame::inviteCurrentUser(const QString &userName)
{
    if (userName.isEmpty()) {
        return;
    }
    // The invite link carries the game's id and, when the game has one, its
    // description (makeGameJoinLink embeds both). Read them back so the prefix
    // names the game by description first, then its id — identical to the
    // context-menu invite so recipients see one consistent message style.
    const QUrl inviteUrlObj(inviteUrl);
    const QUrlQuery inviteQuery(inviteUrlObj);
    const int gameId = inviteQuery.queryItemValue("gameid").toInt();
    const QString gameDescription = inviteQuery.queryItemValue("game");
    const QString prefix = gameDescription.isEmpty()
                               ? tr("Join my game (#%1):").arg(gameId)
                               : tr("Join my game \"%1\" (#%2):").arg(gameDescription).arg(gameId);
    tabSupervisor->sendInviteToUser(userName, prefix + " " + inviteUrl);
    accept();
}

void DlgInviteToGame::retranslateUi()
{
    setWindowTitle(tr("Invite to Game"));
    searchEdit->setPlaceholderText(tr("Search users..."));
    inviteButton->setText(tr("Invite"));
    cancelButton->setText(tr("Cancel"));
    emptyLabel->setText(tr("No matching users."));
}
