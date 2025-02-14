#ifndef DECK_EDITOR_PRINTING_SELECTOR_DOCK_WIDGET_H
#define DECK_EDITOR_PRINTING_SELECTOR_DOCK_WIDGET_H

#include "../../../tabs/tab_generic_deck_editor.h"
#include "../printing_selector/printing_selector.h"

#include <QDockWidget>

class TabDeckEditor;
class DeckEditorPrintingSelectorDockWidget : public QDockWidget
{
    Q_OBJECT
public:
    explicit DeckEditorPrintingSelectorDockWidget(QWidget *parent, TabGenericDeckEditor *_deckEditor);
    void createPrintingSelectorDock();
    void retranslateUi();
    PrintingSelector *printingSelector;

private:
    TabGenericDeckEditor *deckEditor;
};

#endif // DECK_EDITOR_PRINTING_SELECTOR_DOCK_WIDGET_H
