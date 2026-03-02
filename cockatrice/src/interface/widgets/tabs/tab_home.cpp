#include "tab_home.h"

TabHome::TabHome(TabSupervisor *_tabSupervisor, AbstractClient *_client) : Tab(_tabSupervisor), client(_client)
{
    homeWidget = new HomeWidget(this, tabSupervisor);
    setCentralWidget(homeWidget);
}

void TabHome::retranslateUi()
{
}
