/**
 * @file tab_visual_database_display.h
 * @ingroup Tabs
 * @brief TODO: Document this.
 */

#ifndef TAB_VISUAL_DATABASE_DISPLAY_H
#define TAB_VISUAL_DATABASE_DISPLAY_H

#include "../interface/widgets/visual_database_display/visual_database_display_widget.h"
#include "tab.h"

class TabVisualDatabaseDisplay : public Tab
{
    Q_OBJECT

private:
    TabDeckEditor *deckEditor;
    VisualDatabaseDisplayWidget *visualDatabaseDisplayWidget;

public:
    TabVisualDatabaseDisplay(TabSupervisor *_tabSupervisor);
    void retranslateUi() override;
    [[nodiscard]] QString getTabText() const override
    {
        return visualDatabaseDisplayWidget->displayModeButton->isChecked() ? tr("Database Display")
                                                                           : tr("Visual Database Display");
    }
};

#endif // TAB_VISUAL_DATABASE_DISPLAY_H
