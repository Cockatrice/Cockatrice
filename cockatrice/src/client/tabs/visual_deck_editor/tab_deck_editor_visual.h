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
    QWidget *centralWidget;

public:
    explicit TabDeckEditorVisual(TabSupervisor *_tabSupervisor);
    void retranslateUi() override;
    QString getTabText() const override;
    void changeModelIndexAndCardInfo(const ExactCard &activeCard);
    void changeModelIndexToCard(const ExactCard &activeCard);
    void createDeckAnalyticsDock();
    void createMenus() override;
    void createSearchAndDatabaseFrame();
    void createCentralFrame();

public slots:
    void onDeckChanged() override;
    void showPrintingSelector() override;
    void
    processMainboardCardClick(QMouseEvent *event, CardInfoPictureWithTextOverlayWidget *instance, QString zoneName);
    void processCardClickDatabaseDisplay(QMouseEvent *event, CardInfoPictureWithTextOverlayWidget *instance);
    bool actSaveDeckAs() override;
};

#endif
