/**
 * @file deck_editor_filter_dock_widget.h
 * @ingroup DeckEditorWidgets
 * @ingroup CardDatabaseWidgets
 * @brief A Deck Editor DockWidget that displays and allows modifying filters applied to the CardDatabaseDisplayModel.
 */

#ifndef DECK_EDITOR_FILTER_DOCK_WIDGET_H
#define DECK_EDITOR_FILTER_DOCK_WIDGET_H

#include "../../../interface/widgets/tabs/abstract_tab_deck_editor.h"
#include "../../key_signals.h"

#include <QDockWidget>

class FilterTreeModel;
class AbstractTabDeckEditor;
class DeckEditorFilterDockWidget : public QDockWidget
{
    Q_OBJECT
public:
    explicit DeckEditorFilterDockWidget(AbstractTabDeckEditor *parent);
    void createFiltersDock();
    void retranslateUi();
    QAction *aClearFilterAll, *aClearFilterOne;

signals:
    void clearAllDatabaseFilters();

private:
    AbstractTabDeckEditor *deckEditor;
    FilterTreeModel *filterModel;
    QTreeView *filterView;
    KeySignals filterViewKeySignals;
    QWidget *filterBox;

private slots:
    void filterViewCustomContextMenu(const QPoint &point);
    void filterRemove(const QAction *action);
    void actClearFilterAll();
    void actClearFilterOne();
    void refreshShortcuts();
};

#endif // DECK_EDITOR_FILTER_DOCK_WIDGET_H
