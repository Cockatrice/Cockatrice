#include "window_sets.h"
#include "setsmodel.h"
#include "pictureloader.h"
#include "main.h"

#include <QTreeView>
#include <QGridLayout>
#include <QHeaderView>
#include <QPushButton>
#include <QItemSelection>
#include <QMessageBox>
#include <QGroupBox>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QToolBar>
#include <QAction>
#include <QLabel>

WndSets::WndSets(QWidget *parent)
    : QMainWindow(parent)
{
    // left toolbar
    QToolBar *setsEditToolBar = new QToolBar;
    setsEditToolBar->setOrientation(Qt::Vertical);
    setsEditToolBar->setIconSize(QSize(24, 24));
    setsEditToolBar->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    aTop = new QAction(QString(), this);
    aTop->setIcon(QPixmap("theme:icons/arrow_top_green"));
    aTop->setToolTip(tr("Move selected set to the top"));
    aTop->setEnabled(false);
    connect(aTop, SIGNAL(triggered()), this, SLOT(actTop()));
    setsEditToolBar->addAction(aTop);

    aUp = new QAction(QString(), this);
    aUp->setIcon(QPixmap("theme:icons/arrow_up_green"));
    aUp->setToolTip(tr("Move selected set up"));
    aUp->setEnabled(false);
    connect(aUp, SIGNAL(triggered()), this, SLOT(actUp()));
    setsEditToolBar->addAction(aUp);

    aDown = new QAction(QString(), this);
    aDown->setIcon(QPixmap("theme:icons/arrow_down_green"));
    aDown->setToolTip(tr("Move selected set down"));
    aDown->setEnabled(false);
    connect(aDown, SIGNAL(triggered()), this, SLOT(actDown()));
    setsEditToolBar->addAction(aDown);

    aBottom = new QAction(QString(), this);
    aBottom->setIcon(QPixmap("theme:icons/arrow_bottom_green"));
    aBottom->setToolTip(tr("Move selected set to the bottom"));
    aBottom->setEnabled(false);
    connect(aBottom, SIGNAL(triggered()), this, SLOT(actBottom()));
    setsEditToolBar->addAction(aBottom);

    // view 
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

    view->header()->setSectionResizeMode(QHeaderView::Stretch);
    view->header()->setSectionResizeMode(SetsModel::LongNameCol, QHeaderView::ResizeToContents);

    view->sortByColumn(SetsModel::SortKeyCol, Qt::AscendingOrder);
    view->setColumnHidden(SetsModel::SortKeyCol, true);
    view->setColumnHidden(SetsModel::IsKnownCol, true);
    view->setRootIsDecorated(false);

    connect(view->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
        this, SLOT(actToggleButtons(const QItemSelection &, const QItemSelection &)));

    // bottom buttons
    enableAllButton = new QPushButton(tr("Enable all sets"));
    connect(enableAllButton, SIGNAL(clicked()), this, SLOT(actEnableAll()));
    disableAllButton = new QPushButton(tr("Disable all sets"));
    connect(disableAllButton, SIGNAL(clicked()), this, SLOT(actDisableAll()));


    QLabel *labNotes = new QLabel;
    labNotes->setText("<b>" + tr("hints:") + "</b>" + "<ul><li>" + tr("Enable the sets that you want to have available in the deck editor") + "</li><li>" + tr("Move sets around to change their order, or click on a column header to sort sets on that field") + "</li><li>" + tr("Sets order decides the source that will be used when loading images for a specific card") + "</li><li>" + tr("Disabled sets will be used for loading images only if all the enabled sets failed") + "</li></ul>");

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(actSave()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(actRestore()));

    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->addWidget(setsEditToolBar, 0, 0, 1, 1);
    mainLayout->addWidget(view, 0, 1, 1, 2);
    mainLayout->addWidget(enableAllButton, 1, 1, 1, 1);
    mainLayout->addWidget(disableAllButton, 1, 2, 1, 1);
    mainLayout->addWidget(labNotes, 2, 1, 1, 2);
    mainLayout->addWidget(buttonBox, 3, 1, 1, 2);
    mainLayout->setColumnStretch(1, 1);
    mainLayout->setColumnStretch(2, 1);

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
    model->save(db);
    PictureLoader::clearPixmapCache();
    QMessageBox::information(this, tr("Success"), tr("The sets database has been saved successfully."));
    close();
}

void WndSets::actRestore()
{
    model->restore(db);
    close();
}

void WndSets::actToggleButtons(const QItemSelection & selected, const QItemSelection &)
{
    bool disabled = selected.empty();
    aTop->setDisabled(disabled);
    aUp->setDisabled(disabled);
    aDown->setDisabled(disabled);
    aBottom->setDisabled(disabled);
}

void WndSets::selectRow(int row)
{
    QModelIndex idx = model->index(row, 0);
    view->selectionModel()->select(idx, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
    view->scrollTo(idx, QAbstractItemView::EnsureVisible);
}

void WndSets::actEnable()
{
    QModelIndexList rows = view->selectionModel()->selectedRows();
    if(rows.empty())
        return;

    model->toggleRow(rows.first().row(), true);
}

void WndSets::actDisable()
{
    QModelIndexList rows = view->selectionModel()->selectedRows();
    if(rows.empty())
        return;

    model->toggleRow(rows.first().row(), false);
}

void WndSets::actEnableAll()
{
    model->toggleAll(true);
}

void WndSets::actDisableAll()
{
    model->toggleAll(false);
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
