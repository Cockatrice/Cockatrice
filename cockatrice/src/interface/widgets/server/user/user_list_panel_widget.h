/**
 * @file user_list_panel_widget.h
 * @ingroup Lobby
 */

#ifndef COCKATRICE_USER_LIST_PANEL_WIDGET_H
#define COCKATRICE_USER_LIST_PANEL_WIDGET_H

#include "user_list_widget.h"

#include <QWidget>

class AbstractClient;
class QLineEdit;
class TabSupervisor;
class UserListManager;

/**
 * A unified user list: a search bar above a single tree whose section headers
 * (buddy, online, ignored) are inline dividers. The tree owns the scrolling.
 */
class UserListPanelWidget : public QWidget
{
    Q_OBJECT

public:
    explicit UserListPanelWidget(TabSupervisor *tabSupervisor, AbstractClient *client, QWidget *parent = nullptr);
    void bind(UserListManager *manager);
    void retranslateUi();

    [[nodiscard]] UserListWidget *getUserList() const;

signals:
    void openMessageDialog(const QString &userName, bool focus);

private:
    void persistExpandedSections(UserListWidget::Section section, bool expanded);

    QLineEdit *searchBar = nullptr;
    UserListWidget *userList = nullptr;
};

#endif // COCKATRICE_USER_LIST_PANEL_WIDGET_H
