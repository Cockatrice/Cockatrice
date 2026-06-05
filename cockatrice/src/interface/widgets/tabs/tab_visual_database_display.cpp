#include "tab_visual_database_display.h"

#include "tab_deck_editor.h"
#include "tab_supervisor.h"

#include <libcockatrice/card/database/card_database_manager.h>

TabVisualDatabaseDisplay::TabVisualDatabaseDisplay(TabSupervisor *_tabSupervisor) : Tab(_tabSupervisor)
{
    auto databaseModel = new CardDatabaseModel(CardDatabaseManager::getInstance(), true, this);
    databaseModel->setObjectName("databaseModel");

    visualDatabaseDisplayWidget = new VisualDatabaseDisplayWidget(this, databaseModel);

    connect(visualDatabaseDisplayWidget, &VisualDatabaseDisplayWidget::edhrecRequested, this,
            &TabVisualDatabaseDisplay::openEdhrecTab);

    setCentralWidget(visualDatabaseDisplayWidget);

    TabVisualDatabaseDisplay::retranslateUi();
}

void TabVisualDatabaseDisplay::retranslateUi()
{
}

void TabVisualDatabaseDisplay::openEdhrecTab(const CardInfoPtr &info, bool isCommander) const
{
    getTabSupervisor()->addEdhrecTab(info, isCommander);
}