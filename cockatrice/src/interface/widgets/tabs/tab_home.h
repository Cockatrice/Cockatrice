/**
 * @file tab_home.h
 * @ingroup Tabs
 * @brief TODO: Document this.
 */

#ifndef TAB_HOME_H
#define TAB_HOME_H

#include "../interface/widgets/general/home_widget.h"
#include "tab.h"

#include <libcockatrice/network/client/abstract/abstract_client.h>

class AbstractClient;

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
