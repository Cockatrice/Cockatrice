#ifndef WINDOW_DECKEDITORVISUAL_H
#define WINDOW_DECKEDITORVISUAL_H

#include "../tab.h"
#include "tab_deck_editor_visual_tab_widget.h"

class TabDeckEditorVisual : public AbstractTabDeckEditor
{
    Q_OBJECT
protected slots:
    void loadLayout() override;
    void restartLayout() override;
    void freeDocksSize() override;
    void refreshShortcuts() override;

    bool eventFilter(QObject *o, QEvent *e) override;
    void dockVisibleTriggered() override;
    void dockFloatingTriggered() override;
    void dockTopLevelChanged(bool topLevel) override;

protected:
    TabDeckEditorVisualTabWidget *tabContainer;

    QVBoxLayout *centralFrame;
    QVBoxLayout *searchAndDatabaseFrame;
    QHBoxLayout *searchLayout;
    QDockWidget *searchAndDatabaseDock;
    QDockWidget *deckAnalyticsDock;
    QWidget *centralWidget;
    QMenu *deckAnalyticsMenu;
    QAction *aDeckAnalyticsDockVisible, *aDeckAnalyticsDockFloating;

public:
    explicit TabDeckEditorVisual(TabSupervisor *_tabSupervisor);
    void retranslateUi() override;
    QString getTabText() const override;
    void changeModelIndexAndCardInfo(const CardInfoPtr &activeCard);
    void changeModelIndexToCard(const CardInfoPtr &activeCard);
    void createDeckAnalyticsDock();
    void createMenus() override;
    void createSearchAndDatabaseFrame();
    void createCentralFrame();

public slots:
    void onDeckChanged() override;
    void showPrintingSelector() override;
    void processMainboardCardClick(QMouseEvent *event, CardInfoPictureWithTextOverlayWidget *instance);
    void processSideboardCardClick(QMouseEvent *event, CardInfoPictureWithTextOverlayWidget *instance);
    void processCardClickDatabaseDisplay(QMouseEvent *event, CardInfoPictureWithTextOverlayWidget *instance);
};

#endif
