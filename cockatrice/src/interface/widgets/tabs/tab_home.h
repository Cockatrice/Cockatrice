/**
 * @file tab_home.h
 * @ingroup Tabs
 */
//! \todo Document this file.

#ifndef TAB_HOME_H
#define TAB_HOME_H

#include "tab.h"

#include <QString>
#include <qtmetamacros.h>

class AbstractClient;
class HomeWidget;
class TabSupervisor;

class TabHome : public Tab
{
    Q_OBJECT
private:
    AbstractClient *client;
    HomeWidget *homeWidget;

public:
    TabHome(TabSupervisor *_tabSupervisor, AbstractClient *_client);
    void retranslateUi() override;
    [[nodiscard]] QString getTabText() const override
    {
        return tr("Home");
    }
};

#endif // TAB_HOME_H
