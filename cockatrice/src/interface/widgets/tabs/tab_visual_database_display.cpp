#include "tab_visual_database_display.h"

#include "../interface/widgets/deck_editor/deck_editor_card_database_dock_widget.h"
#include "../interface/widgets/deck_editor/deck_editor_database_display_widget.h"
#include "../interface/widgets/visual_database_display/visual_database_display_widget.h"
#include "tab_deck_editor.h"

#include <QPushButton>

TabVisualDatabaseDisplay::TabVisualDatabaseDisplay(TabSupervisor *_tabSupervisor) : Tab(_tabSupervisor)
{
    deckEditor = new TabDeckEditor(_tabSupervisor);
    deckEditor->setHidden(true);
    visualDatabaseDisplayWidget = new VisualDatabaseDisplayWidget(
        this, deckEditor, deckEditor->cardDatabaseDockWidget->databaseDisplayWidget->databaseModel,
        deckEditor->cardDatabaseDockWidget->databaseDisplayWidget->databaseDisplayModel);

    setCentralWidget(visualDatabaseDisplayWidget);

    TabVisualDatabaseDisplay::retranslateUi();
}

void TabVisualDatabaseDisplay::retranslateUi()
{
}
QString TabVisualDatabaseDisplay::getTabText() const
{
    return visualDatabaseDisplayWidget->displayModeButton->isChecked() ? tr("Database Display")
                                                                       : tr("Visual Database Display");
}
