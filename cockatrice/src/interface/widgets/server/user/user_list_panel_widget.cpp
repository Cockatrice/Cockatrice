#include "user_list_panel_widget.h"

#include "../../../../client/settings/cache_settings.h"
#include "user_list_manager.h"
#include "user_list_widget.h"

#include <QLineEdit>
#include <QVBoxLayout>
#include <libcockatrice/network/client/abstract/abstract_client.h>
#include <libcockatrice/settings/interface_settings.h>

UserListPanelWidget::UserListPanelWidget(TabSupervisor *_tabSupervisor, AbstractClient *_client, QWidget *parent)
    : QWidget(parent)
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(2);

    searchBar = new QLineEdit(this);
    searchBar->setClearButtonEnabled(true);
    mainLayout->addWidget(searchBar);

    userList = new UserListWidget(_tabSupervisor, _client, UserListWidget::RoomList, this);
    userList->setSectioned({QStringLiteral("buddy"), QStringLiteral("online"), QStringLiteral("ignore")});
    mainLayout->addWidget(userList, 1);

    connect(searchBar, &QLineEdit::textChanged, userList, &UserListWidget::setFilterText);

    connect(userList, &UserListWidget::sectionExpanded, this, &UserListPanelWidget::persistExpandedSections);
    connect(userList, &UserListWidget::openMessageDialog, this, &UserListPanelWidget::openMessageDialog);

    // Restore the persisted expansion state, then apply it to the tree.
    const QStringList expandedSections = SettingsCache::instance().userInterface().getUserListExpandedSections();
    for (const QString &sectionId : userList->getSectionIds()) {
        userList->setSectionExpanded(sectionId, expandedSections.contains(sectionId));
    }

    retranslateUi();
}

void UserListPanelWidget::bind(UserListManager *manager)
{
    userList->bind(manager);
}

void UserListPanelWidget::persistExpandedSections(const QString &sectionId, bool expanded)
{
    QStringList expandedSections = SettingsCache::instance().userInterface().getUserListExpandedSections();
    if (expanded) {
        if (!expandedSections.contains(sectionId)) {
            expandedSections.append(sectionId);
        }
    } else {
        expandedSections.removeAll(sectionId);
    }
    SettingsCache::instance().userInterface().setUserListExpandedSections(expandedSections);
}

void UserListPanelWidget::retranslateUi()
{
    searchBar->setPlaceholderText(tr("Search users..."));
    userList->retranslateUi();
}

UserListWidget *UserListPanelWidget::getUserList() const
{
    return userList;
}
