/**
 * @file tab_visual_database_display.h
 * @ingroup Tabs
 */
//! \todo Document this file.

#ifndef TAB_VISUAL_DATABASE_DISPLAY_H
#define TAB_VISUAL_DATABASE_DISPLAY_H

#include "../interface/widgets/visual_database_display/visual_database_display_widget.h"
#include "libcockatrice/card/card_info.h"
#include "tab.h"

#include <QString>
#include <qtmetamacros.h>

class TabSupervisor;
class CardInfoFrameWidget;
class QDockWidget;
class QMenu;
class QAction;
class QLabel;
class QHideEvent;

class TabVisualDatabaseDisplay : public Tab
{
    Q_OBJECT

private:
    VisualDatabaseDisplayWidget *visualDatabaseDisplayWidget;
    CardInfoFrameWidget *cardInfo;
    QDockWidget *cardInfoDock;
    QMenu *viewMenu;
    QAction *resetLayoutAction;
    QLabel *selectionHint;

    void inspectCard(const ExactCard &card);
    void resetLayout();

protected:
    void hideEvent(QHideEvent *event) override;

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
