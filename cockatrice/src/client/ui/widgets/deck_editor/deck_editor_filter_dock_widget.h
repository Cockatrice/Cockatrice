#ifndef DECK_EDITOR_FILTER_DOCK_WIDGET_H
#define DECK_EDITOR_FILTER_DOCK_WIDGET_H

#include "../../../game_logic/key_signals.h"
#include "../../../tabs/abstract_tab_deck_editor.h"

#include <QDockWidget>
#include <QTreeView>

class FilterTreeModel;
class AbstractTabDeckEditor;
class DeckEditorFilterDockWidget : public QDockWidget
{
    Q_OBJECT
public:
    explicit DeckEditorFilterDockWidget(QWidget *parent, AbstractTabDeckEditor *_deckEditor);
    void createFiltersDock();
    void retranslateUi();
    QAction *aClearFilterAll, *aClearFilterOne;

private:
    AbstractTabDeckEditor *deckEditor;
    FilterTreeModel *filterModel;
    QTreeView *filterView;
    KeySignals filterViewKeySignals;
    QWidget *filterBox;

    void filterRemove(QAction *action);

private slots:
    void filterViewCustomContextMenu(const QPoint &point);
    void actClearFilterAll();
    void actClearFilterOne();
    void refreshShortcuts();
};

#endif // DECK_EDITOR_FILTER_DOCK_WIDGET_H
