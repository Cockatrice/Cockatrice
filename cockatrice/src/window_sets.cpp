#include "window_sets.h"
#include "setsmodel.h"
#include "main.h"
#include <QTreeView>
#include <QGridLayout>
#include <QHeaderView>
#include <QPushButton>
#include <QItemSelection>
#include <QMessageBox>

WndSets::WndSets(QWidget *parent)
    : QMainWindow(parent)
{
    model = new SetsModel(db, this);
    view = new QTreeView;
    view->setModel(model);

    view->setAlternatingRowColors(true);
    view->setUniformRowHeights(true);
    view->setAllColumnsShowFocus(true);
    view->setSortingEnabled(true);
    view->setSelectionMode(QAbstractItemView::SingleSelection);
    view->setSelectionBehavior(QAbstractItemView::SelectRows);

    view->setDragEnabled(true);
    view->setAcceptDrops(true);
    view->setDropIndicatorShown(true);
    view->setDragDropMode(QAbstractItemView::InternalMove);

#if QT_VERSION < 0x050000
    view->header()->setResizeMode(QHeaderView::Stretch);
    view->header()->setResizeMode(SetsModel::LongNameCol, QHeaderView::ResizeToContents);
#else
    view->header()->setSectionResizeMode(QHeaderView::Stretch);
    view->header()->setSectionResizeMode(SetsModel::LongNameCol, QHeaderView::ResizeToContents);
#endif

    view->sortByColumn(SetsModel::SortKeyCol, Qt::AscendingOrder);
    view->setColumnHidden(SetsModel::SortKeyCol, true);

    saveButton = new QPushButton(tr("Save set ordering"));
    connect(saveButton, SIGNAL(clicked()), this, SLOT(actSave()));
    restoreButton = new QPushButton(tr("Restore saved set ordering"));
    connect(restoreButton, SIGNAL(clicked()), this, SLOT(actRestore()));
    upButton = new QPushButton(tr("Move selected set up"));
    connect(upButton, SIGNAL(clicked()), this, SLOT(actUp()));
    downButton = new QPushButton(tr("Move selected set down"));
    connect(downButton, SIGNAL(clicked()), this, SLOT(actDown()));
    topButton = new QPushButton(tr("Move selected set to top"));
    connect(topButton, SIGNAL(clicked()), this, SLOT(actTop()));
    bottomButton = new QPushButton(tr("Move selected set to bottom"));
    connect(bottomButton, SIGNAL(clicked()), this, SLOT(actBottom()));

    upButton->setDisabled(true);
    downButton->setDisabled(true);
    topButton->setDisabled(true);
    bottomButton->setDisabled(true);

    connect(view->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
        this, SLOT(actToggleButtons(const QItemSelection &, const QItemSelection &)));

    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->addWidget(view, 0, 0, 1, 2);

    mainLayout->addWidget(upButton, 1, 0, 1, 1);
    mainLayout->addWidget(downButton, 2, 0, 1, 1);

    mainLayout->addWidget(topButton, 1, 1, 1, 1);
    mainLayout->addWidget(bottomButton, 2, 1, 1, 1);

    mainLayout->addWidget(saveButton, 3, 0, 1, 1);
    mainLayout->addWidget(restoreButton, 3, 1, 1, 1);

    QWidget *centralWidget = new QWidget;
    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);

    setWindowTitle(tr("Edit sets"));
    resize(700, 400);
}

WndSets::~WndSets()
{
}

void WndSets::actSave()
{
    model->save();
    QMessageBox::information(this, tr("Success"), tr("The sets database has been saved successfully."));
}

void WndSets::actRestore()
{
    model->restore(db);
}

void WndSets::actToggleButtons(const QItemSelection & selected, const QItemSelection &)
{
    bool disabled = selected.empty();
    upButton->setDisabled(disabled);
    downButton->setDisabled(disabled);
    topButton->setDisabled(disabled);
    bottomButton->setDisabled(disabled);
}

void WndSets::selectRow(int row)
{
    QModelIndex idx = model->index(row, 0);
    view->selectionModel()->select(idx, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
    view->scrollTo(idx, QAbstractItemView::EnsureVisible);
}

void WndSets::actUp()
{
    QModelIndexList rows = view->selectionModel()->selectedRows();
    if(rows.empty())
        return;

    QModelIndex selectedRow = rows.first();
    int oldRow = selectedRow.row();
    int newRow = oldRow - 1;
    if(oldRow <= 0)
        return;

    model->swapRows(oldRow, newRow);
    selectRow(newRow);
}

void WndSets::actDown()
{
    QModelIndexList rows = view->selectionModel()->selectedRows();
    if(rows.empty())
        return;

    QModelIndex selectedRow = rows.first();
    int oldRow = selectedRow.row();
    int newRow = oldRow + 1;
    if(oldRow >= model->rowCount() - 1)
        return;

    model->swapRows(oldRow, newRow);
    selectRow(newRow);
}

void WndSets::actTop()
{
    QModelIndexList rows = view->selectionModel()->selectedRows();
    if(rows.empty())
        return;

    QModelIndex selectedRow = rows.first();
    int oldRow = selectedRow.row();
    int newRow = 0;
    if(oldRow <= 0)
        return;

    model->swapRows(oldRow, newRow);
    selectRow(newRow);
}

void WndSets::actBottom()
{
    QModelIndexList rows = view->selectionModel()->selectedRows();
    if(rows.empty())
        return;

    QModelIndex selectedRow = rows.first();
    int oldRow = selectedRow.row();
    int newRow = model->rowCount() - 1;
    if(oldRow >= newRow)
        return;

    model->swapRows(oldRow, newRow);
    selectRow(newRow);
}
