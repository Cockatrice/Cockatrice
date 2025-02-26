#include "deck_editor_filter_dock_widget.h"

#include "../../../../game/cards/card_database_model.h"
#include "../../../../game/filters/filter_builder.h"
#include "../../../../game/filters/filter_tree_model.h"
#include "../../../../settings/cache_settings.h"

#include <QGridLayout>
#include <QMenu>
#include <QToolButton>

DeckEditorFilterDockWidget::DeckEditorFilterDockWidget(QWidget *parent, TabGenericDeckEditor *_deckEditor)
    : QDockWidget(parent), deckEditor(_deckEditor)
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
    connect(filterModel, SIGNAL(layoutChanged()), filterView, SLOT(expandAll()));
    connect(filterView, SIGNAL(customContextMenuRequested(const QPoint &)), this,
            SLOT(filterViewCustomContextMenu(const QPoint &)));
    connect(&filterViewKeySignals, SIGNAL(onDelete()), this, SLOT(actClearFilterOne()));

    auto *filterBuilder = new FilterBuilder;
    filterBuilder->setObjectName("filterBuilder");
    connect(filterBuilder, SIGNAL(add(const CardFilter *)), filterModel, SLOT(addFilter(const CardFilter *)));

    aClearFilterOne = new QAction(QString(), this);
    aClearFilterOne->setIcon(QPixmap("theme:icons/decrement"));
    connect(aClearFilterOne, SIGNAL(triggered()), this, SLOT(actClearFilterOne()));

    aClearFilterAll = new QAction(QString(), this);
    aClearFilterAll->setIcon(QPixmap("theme:icons/clearsearch"));
    connect(aClearFilterAll, SIGNAL(triggered()), this, SLOT(actClearFilterAll()));

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
    connect(this, &QDockWidget::topLevelChanged, deckEditor, &TabGenericDeckEditor::dockTopLevelChanged);
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
    connect(&menu, SIGNAL(triggered(QAction *)), this, SLOT(filterRemove(QAction *)));
    menu.exec(filterView->mapToGlobal(point));
}

void DeckEditorFilterDockWidget::filterRemove(QAction *action)
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
    deckEditor->databaseDisplayDockWidget->databaseDisplayModel->clearFilterAll();
    deckEditor->databaseDisplayDockWidget->searchEdit->setText("");
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