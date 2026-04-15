/**
 * @file tab_visual_database_display.h
 * @ingroup Tabs
 * @brief TODO: Document this.
 */

#ifndef TAB_VISUAL_DATABASE_DISPLAY_H
#define TAB_VISUAL_DATABASE_DISPLAY_H

#include "tab.h"

class VisualDatabaseDisplayWidget;
class TabDeckEditor;
class TabVisualDatabaseDisplay : public Tab
{
    Q_OBJECT

private:
    TabDeckEditor *deckEditor;
    VisualDatabaseDisplayWidget *visualDatabaseDisplayWidget;

public:
    TabVisualDatabaseDisplay(TabSupervisor *_tabSupervisor);
    void retranslateUi() override;
    [[nodiscard]] QString getTabText() const override;
};

#endif // TAB_VISUAL_DATABASE_DISPLAY_H
