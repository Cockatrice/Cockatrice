#ifndef TAB_DECK_EDITOR_VISUAL_TAB_WIDGET_H
#define TAB_DECK_EDITOR_VISUAL_TAB_WIDGET_H

#include "../../ui/widgets/deck_analytics/deck_analytics_widget.h"
#include "../../ui/widgets/printing_selector/printing_selector.h"
#include "../../ui/widgets/visual_database_display/visual_database_display_widget.h"
#include "../../ui/widgets/visual_deck_editor/visual_deck_editor_sample_hand_widget.h"
#include "../../ui/widgets/visual_deck_editor/visual_deck_editor_widget.h"
#include "../tab_generic_deck_editor.h"

#include <QTabWidget>
#include <QVBoxLayout>
#include <QWidget>

class TabDeckEditorVisualTabWidget : public QTabWidget
{
    Q_OBJECT

public:
    explicit TabDeckEditorVisualTabWidget(QWidget *parent = nullptr,
                                          TabGenericDeckEditor *deckEditor = nullptr,
                                          DeckListModel *deckModel = nullptr,
                                          CardDatabaseModel *card_database_model = nullptr,
                                          CardDatabaseDisplayModel *card_database_display_model = nullptr);

    // Utility functions
    void addNewTab(QWidget *widget, const QString &title);
    void removeCurrentTab();
    void setTabTitle(int index, const QString &title);
    QWidget *getCurrentTab() const;
    int getTabCount() const;

    VisualDeckEditorWidget *visualDeckView;
    DeckAnalyticsWidget *deckAnalytics;
    VisualDatabaseDisplayWidget *visualDatabaseDisplay;
    PrintingSelector *printingSelector;
    VisualDeckEditorSampleHandWidget *sampleHandWidget;

public slots:
    void onCardChanged(CardInfoPtr activeCard);
    void onCardChangedDatabaseDisplay(CardInfoPtr activeCard);
    void onMainboardCardClickedDeckEditor(QMouseEvent *event, CardInfoPictureWithTextOverlayWidget *instance);
    void onSideboardCardClickedDeckEditor(QMouseEvent *event, CardInfoPictureWithTextOverlayWidget *instance);
    void onCardClickedDatabaseDisplay(QMouseEvent *event, CardInfoPictureWithTextOverlayWidget *instance);

signals:
    void cardChanged(CardInfoPtr activeCard);
    void cardChangedDatabaseDisplay(CardInfoPtr activeCard);
    void mainboardCardClicked(QMouseEvent *event, CardInfoPictureWithTextOverlayWidget *instance);
    void sideboardCardClicked(QMouseEvent *event, CardInfoPictureWithTextOverlayWidget *instance);
    void cardClickedDatabaseDisplay(QMouseEvent *event, CardInfoPictureWithTextOverlayWidget *instance);

private:
    QVBoxLayout *layout; // Layout for the tab widget and other controls
    TabGenericDeckEditor *deckEditor;
    DeckListModel *deckModel;
    CardDatabaseModel *cardDatabaseModel;
    CardDatabaseDisplayModel *cardDatabaseDisplayModel;

private slots:
    void handleTabClose(int index); // Slot for closing a tab
};

#endif // TAB_DECK_EDITOR_VISUAL_TAB_WIDGET_H
