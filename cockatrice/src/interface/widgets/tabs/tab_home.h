/**
 * @file tab_home.h
 * @ingroup Tabs
 * @brief TODO: Document this.
 */

#ifndef TAB_HOME_H
#define TAB_HOME_H

#include "tab.h"

class AbstractClient;
class HomeWidget;
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
