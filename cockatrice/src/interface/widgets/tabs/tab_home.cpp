#include "tab_home.h"

#include "../general/home_widget.h"

class TabSupervisor;

TabHome::TabHome(TabSupervisor *_tabSupervisor, AbstractClient *_client) : Tab(_tabSupervisor), client(_client)
{
    homeWidget = new HomeWidget(this, tabSupervisor);
    setCentralWidget(homeWidget);
}

void TabHome::retranslateUi()
{
}
