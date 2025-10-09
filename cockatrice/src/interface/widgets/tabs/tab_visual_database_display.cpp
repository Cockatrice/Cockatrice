#include "tab_visual_database_display.h"

#include "tab_deck_editor.h"

TabVisualDatabaseDisplay::TabVisualDatabaseDisplay(TabSupervisor *_tabSupervisor) : Tab(_tabSupervisor)
{
    deckEditor = new TabDeckEditor(_tabSupervisor);
    deckEditor->setHidden(true);
    visualDatabaseDisplayWidget =
        new VisualDatabaseDisplayWidget(this, deckEditor, deckEditor->databaseDisplayDockWidget->databaseModel,
                                        deckEditor->databaseDisplayDockWidget->databaseDisplayModel);

    setCentralWidget(visualDatabaseDisplayWidget);

    TabVisualDatabaseDisplay::retranslateUi();
}

void TabVisualDatabaseDisplay::retranslateUi()
{
}
