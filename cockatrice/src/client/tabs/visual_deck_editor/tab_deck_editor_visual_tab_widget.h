#ifndef TAB_DECK_EDITOR_VISUAL_TAB_WIDGET_H
#define TAB_DECK_EDITOR_VISUAL_TAB_WIDGET_H

#include "../../ui/widgets/printing_selector/printing_selector.h"
#include "../../ui/widgets/visual_database_display/visual_database_display_widget.h"
#include "../../ui/widgets/visual_deck_editor/visual_deck_editor_widget.h"
#include "../abstract_tab_deck_editor.h"

#include <QTabWidget>
#include <QVBoxLayout>
#include <QWidget>

class TabDeckEditorVisualTabWidget : public QTabWidget
{
    Q_OBJECT

public:
    explicit TabDeckEditorVisualTabWidget(QWidget *parent,
                                          AbstractTabDeckEditor *_deckEditor,
                                          DeckListModel *_deckModel,
                                          CardDatabaseModel *_cardDatabaseModel,
                                          CardDatabaseDisplayModel *_cardDatabaseDisplayModel);

    // Utility functions
    void addNewTab(QWidget *widget, const QString &title);
    void removeCurrentTab();
    void setTabTitle(int index, const QString &title);
    QWidget *getCurrentTab() const;
    int getTabCount() const;

    VisualDeckEditorWidget *visualDeckView;
    VisualDatabaseDisplayWidget *visualDatabaseDisplay;
    PrintingSelector *printingSelector;

public slots:
    void onCardChanged(CardInfoPtr activeCard);
    void onCardChangedDatabaseDisplay(CardInfoPtr activeCard);
    void onCardClickedDeckEditor(QMouseEvent *event, CardInfoPictureWithTextOverlayWidget *instance, QString zoneName);
    void onCardClickedDatabaseDisplay(QMouseEvent *event, CardInfoPictureWithTextOverlayWidget *instance);

signals:
    void cardChanged(CardInfoPtr activeCard);
    void cardChangedDatabaseDisplay(CardInfoPtr activeCard);
    void cardClicked(QMouseEvent *event, CardInfoPictureWithTextOverlayWidget *instance, QString zoneName);
    void cardClickedDatabaseDisplay(QMouseEvent *event, CardInfoPictureWithTextOverlayWidget *instance);

private:
    QVBoxLayout *layout; // Layout for the tab widget and other controls
    AbstractTabDeckEditor *deckEditor;
    DeckListModel *deckModel;
    CardDatabaseModel *cardDatabaseModel;
    CardDatabaseDisplayModel *cardDatabaseDisplayModel;

private slots:
    void handleTabClose(int index); // Slot for closing a tab
};

#endif // TAB_DECK_EDITOR_VISUAL_TAB_WIDGET_H
