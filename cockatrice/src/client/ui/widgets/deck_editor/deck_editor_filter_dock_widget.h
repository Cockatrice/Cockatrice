#ifndef DECK_EDITOR_FILTER_DOCK_WIDGET_H
#define DECK_EDITOR_FILTER_DOCK_WIDGET_H

#include "../../../game_logic/key_signals.h"
#include "../../../tabs/tab_generic_deck_editor.h"

#include <QDockWidget>
#include <QTreeView>

class FilterTreeModel;
class TabGenericDeckEditor;
class DeckEditorFilterDockWidget : public QDockWidget
{
    Q_OBJECT
public:
    explicit DeckEditorFilterDockWidget(QWidget *parent, TabGenericDeckEditor *_deckEditor);
    void createFiltersDock();
    void retranslateUi();
    QAction *aClearFilterAll, *aClearFilterOne;

private:
    TabGenericDeckEditor *deckEditor;
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
