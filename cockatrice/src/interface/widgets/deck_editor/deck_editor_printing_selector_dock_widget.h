/**
 * @file deck_editor_printing_selector_dock_widget.h
 * @ingroup DeckEditorWidgets
 * @brief A Deck Editor DockWidget that displays a PrintingSelector, which allows choosing different printings.
 */

#ifndef DECK_EDITOR_PRINTING_SELECTOR_DOCK_WIDGET_H
#define DECK_EDITOR_PRINTING_SELECTOR_DOCK_WIDGET_H

#include <QDockWidget>

class AbstractTabDeckEditor;
class PrintingSelector;
class PrintingDisabledInfoWidget;
class TabDeckEditor;

class DeckEditorPrintingSelectorDockWidget : public QDockWidget
{
    Q_OBJECT
public:
    explicit DeckEditorPrintingSelectorDockWidget(AbstractTabDeckEditor *parent);
    void createPrintingSelectorDock();
    void retranslateUi();

    PrintingSelector *printingSelector;

private:
    AbstractTabDeckEditor *deckEditor;
    QWidget *printingSelectorDockContents;
    PrintingDisabledInfoWidget *printingDisabledInfoWidget;

private slots:
    void setVisibleWidget(bool overridePrintings);
};

#endif // DECK_EDITOR_PRINTING_SELECTOR_DOCK_WIDGET_H
