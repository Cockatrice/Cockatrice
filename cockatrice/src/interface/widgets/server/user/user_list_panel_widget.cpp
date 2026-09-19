#include "user_list_panel_widget.h"

#include "../../../../client/settings/cache_settings.h"
#include "user_list_manager.h"
#include "user_list_widget.h"

#include <QLineEdit>
#include <QVBoxLayout>
#include <libcockatrice/network/client/abstract/abstract_client.h>
#include <libcockatrice/settings/interface_settings.h>

namespace
{
// The persisted section keys are the serialization contract with the user's
// settings file, so the values must stay stable across versions.
QString sectionKey(UserListWidget::Section section)
{
    switch (section) {
        case UserListWidget::Section::Buddy:
            return QStringLiteral("buddy");
        case UserListWidget::Section::Online:
            return QStringLiteral("online");
        case UserListWidget::Section::Ignore:
            return QStringLiteral("ignore");
    }
    return {};
}
} // namespace

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
    userList->setSectioned(
        {UserListWidget::Section::Buddy, UserListWidget::Section::Online, UserListWidget::Section::Ignore});
    mainLayout->addWidget(userList, 1);

    connect(searchBar, &QLineEdit::textChanged, userList, &UserListWidget::setFilterText);

    connect(userList, &UserListWidget::sectionExpanded, this, &UserListPanelWidget::persistExpandedSections);
    connect(userList, &UserListWidget::openMessageDialog, this, &UserListPanelWidget::openMessageDialog);

    // Restore the persisted expansion state, then apply it to the tree.
    const QStringList expandedSections = SettingsCache::instance().userInterface().getUserListExpandedSections();
    for (const UserListWidget::Section section : userList->getSectionIds()) {
        userList->setSectionExpanded(section, expandedSections.contains(sectionKey(section)));
    }

    retranslateUi();
}

void UserListPanelWidget::bind(UserListManager *manager)
{
    userList->bind(manager);
}

void UserListPanelWidget::persistExpandedSections(UserListWidget::Section section, bool expanded)
{
    const QString key = sectionKey(section);
    QStringList expandedSections = SettingsCache::instance().userInterface().getUserListExpandedSections();
    if (expanded) {
        if (!expandedSections.contains(key)) {
            expandedSections.append(key);
        }
    } else {
        expandedSections.removeAll(key);
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
