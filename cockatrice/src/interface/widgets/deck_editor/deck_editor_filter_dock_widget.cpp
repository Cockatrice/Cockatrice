#include "deck_editor_filter_dock_widget.h"

#include "../../../client/settings/cache_settings.h"
#include "../../../filters/filter_builder.h"
#include "../../../filters/filter_tree_model.h"

#include <QGridLayout>
#include <QMenu>
#include <QToolButton>

DeckEditorFilterDockWidget::DeckEditorFilterDockWidget(AbstractTabDeckEditor *parent)
    : QDockWidget(parent), deckEditor(parent)
{
    setObjectName("filterDock");

    setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable);

    createFiltersDock();
    retranslateUi();
}

void DeckEditorFilterDockWidget::createFiltersDock()
{
    filterModel = new FilterTreeModel();
    filterModel->setObjectName("filterModel");
    deckEditor->filterTreeChanged(filterModel->filterTree());
    filterView = new QTreeView;
    filterView->setObjectName("filterView");
    filterView->setModel(filterModel);
    filterView->setUniformRowHeights(true);
    filterView->setHeaderHidden(true);
    filterView->setContextMenuPolicy(Qt::CustomContextMenu);
    filterView->installEventFilter(&filterViewKeySignals);
    connect(filterModel, &FilterTreeModel::layoutChanged, filterView, &QTreeView::expandAll);
    connect(filterView, &QTreeView::customContextMenuRequested, this,
            &DeckEditorFilterDockWidget::filterViewCustomContextMenu);
    connect(&filterViewKeySignals, &KeySignals::onDelete, this, &DeckEditorFilterDockWidget::actClearFilterOne);

    auto *filterBuilder = new FilterBuilder;
    filterBuilder->setObjectName("filterBuilder");
    connect(filterBuilder, &FilterBuilder::add, filterModel, &FilterTreeModel::addFilter);

    aClearFilterOne = new QAction(QString(), this);
    aClearFilterOne->setIcon(QPixmap("theme:icons/decrement"));
    connect(aClearFilterOne, &QAction::triggered, this, &DeckEditorFilterDockWidget::actClearFilterOne);

    aClearFilterAll = new QAction(QString(), this);
    aClearFilterAll->setIcon(QPixmap("theme:icons/clearsearch"));
    connect(aClearFilterAll, &QAction::triggered, this, &DeckEditorFilterDockWidget::actClearFilterAll);

    auto *filterDelOne = new QToolButton();
    filterDelOne->setObjectName("filterDelOne");
    filterDelOne->setDefaultAction(aClearFilterOne);
    filterDelOne->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    auto *filterDelAll = new QToolButton();
    filterDelAll->setObjectName("filterDelAll");
    filterDelAll->setDefaultAction(aClearFilterAll);
    filterDelAll->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    auto *filterLayout = new QGridLayout;
    filterLayout->setObjectName("filterLayout");
    filterLayout->setContentsMargins(0, 0, 0, 0);
    filterLayout->addWidget(filterBuilder, 0, 0, 1, 3);
    filterLayout->addWidget(filterView, 1, 0, 1, 3);
    filterLayout->addWidget(filterDelOne, 2, 0, 1, 1);
    filterLayout->addWidget(filterDelAll, 2, 2, 1, 1);

    filterBox = new QWidget();
    filterBox->setObjectName("filterBox");
    filterBox->setLayout(filterLayout);

    auto *filterFrame = new QVBoxLayout;
    filterFrame->setObjectName("filterFrame");
    filterFrame->addWidget(filterBox);

    auto *filterDockContents = new QWidget(this);
    filterDockContents->setObjectName("filterDockContents");
    filterDockContents->setLayout(filterFrame);
    setWidget(filterDockContents);

    installEventFilter(deckEditor);
    connect(this, &QDockWidget::topLevelChanged, deckEditor, &AbstractTabDeckEditor::dockTopLevelChanged);
}

void DeckEditorFilterDockWidget::filterViewCustomContextMenu(const QPoint &point)
{
    QMenu menu;
    QAction *action;
    QModelIndex idx;

    idx = filterView->indexAt(point);
    if (!idx.isValid())
        return;

    action = menu.addAction(QString("delete"));
    action->setData(point);
    connect(&menu, &QMenu::triggered, this, &DeckEditorFilterDockWidget::filterRemove);
    menu.exec(filterView->mapToGlobal(point));
}

void DeckEditorFilterDockWidget::filterRemove(const QAction *action)
{
    QPoint point;
    QModelIndex idx;

    point = action->data().toPoint();
    idx = filterView->indexAt(point);
    if (!idx.isValid())
        return;

    filterModel->removeRow(idx.row(), idx.parent());
}

void DeckEditorFilterDockWidget::actClearFilterAll()
{
    emit clearAllDatabaseFilters();
}

void DeckEditorFilterDockWidget::actClearFilterOne()
{
    QModelIndexList selIndexes = filterView->selectionModel()->selectedIndexes();
    for (QModelIndex idx : selIndexes) {
        filterModel->removeRow(idx.row(), idx.parent());
    }
}

void DeckEditorFilterDockWidget::refreshShortcuts()
{
    ShortcutsSettings &shortcuts = SettingsCache::instance().shortcuts();

    aClearFilterAll->setShortcuts(shortcuts.getShortcut("TabDeckEditor/aClearFilterAll"));
    aClearFilterOne->setShortcuts(shortcuts.getShortcut("TabDeckEditor/aClearFilterOne"));
}

void DeckEditorFilterDockWidget::retranslateUi()
{
    setWindowTitle(tr("Filters"));

    aClearFilterAll->setText(tr("&Clear all filters"));
    aClearFilterOne->setText(tr("Delete selected"));
}