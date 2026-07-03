#ifndef COCKATRICE_DECK_EDITOR_CARD_DATABASE_DOCK_WIDGET_H
#define COCKATRICE_DECK_EDITOR_CARD_DATABASE_DOCK_WIDGET_H

#include "../../../interface/widgets/tabs/abstract_tab_deck_editor.h"

#include <QDockWidget>

class AbstractTabDeckEditor;
class CardDatabase;
class DeckEditorDatabaseDisplayWidget;
class FilterTree;

class DeckEditorCardDatabaseDockWidget : public QDockWidget
{
public:
    explicit DeckEditorCardDatabaseDockWidget(AbstractTabDeckEditor *parent);

    DeckEditorDatabaseDisplayWidget *databaseDisplayWidget;

    void setFilterTree(FilterTree *filterTree);

public slots:
    void retranslateUi();
    void clearAllDatabaseFilters();

private:
    void createDatabaseDisplayDock(AbstractTabDeckEditor *deckEditor);
};

#endif // COCKATRICE_DECK_EDITOR_CARD_DATABASE_DOCK_WIDGET_H
