/**
 * @file tab_visual_database_display.h
 * @ingroup Tabs
 */
//! \todo Document this file.

#ifndef TAB_VISUAL_DATABASE_DISPLAY_H
#define TAB_VISUAL_DATABASE_DISPLAY_H

#include "../interface/widgets/visual_database_display/visual_database_display_widget.h"
#include "tab.h"

class TabVisualDatabaseDisplay : public Tab
{
    Q_OBJECT

private:
    VisualDatabaseDisplayWidget *visualDatabaseDisplayWidget;

private slots:
    void openEdhrecTab(const CardInfoPtr &info, bool isCommander) const;

public:
    TabVisualDatabaseDisplay(TabSupervisor *_tabSupervisor);
    void retranslateUi() override;
    [[nodiscard]] QString getTabText() const override
    {
        return visualDatabaseDisplayWidget->isVisualDisplayMode() ? tr("Visual Database Display")
                                                                  : tr("Database Display");
    }
};

#endif // TAB_VISUAL_DATABASE_DISPLAY_H
