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
#include <QDebug>

WndSets::WndSets(QWidget *parent)
    : QMainWindow(parent)
{
    // left toolbar
    setsEditToolBar = new QToolBar;
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
    view->setSelectionMode(QAbstractItemView::ExtendedSelection);

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

    // bottom buttons
    enableAllButton = new QPushButton(tr("Enable all sets"));
    disableAllButton = new QPushButton(tr("Disable all sets"));
    enableSomeButton = new QPushButton(tr("Enable selected set(s)"));
    disableSomeButton = new QPushButton(tr("Disable selected set(s)"));

    connect(enableAllButton, SIGNAL(clicked()), this, SLOT(actEnableAll()));
    connect(disableAllButton, SIGNAL(clicked()), this, SLOT(actDisableAll()));
    connect(enableSomeButton, SIGNAL(clicked()), this, SLOT(actEnableSome()));
    connect(disableSomeButton, SIGNAL(clicked()), this, SLOT(actDisableSome()));
    connect(view->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
            this, SLOT(actToggleButtons(const QItemSelection &, const QItemSelection &)));

    labNotes = new QLabel;
    labNotes->setWordWrap(true);
    labNotes->setTextInteractionFlags(Qt::TextBrowserInteraction);
    labNotes->setOpenExternalLinks(true);
    labNotes->setText(
    "<b>" + tr("Deck Editor") + ":</b> "
    + tr("Only cards in enabled sets will appear in the deck editor card list")
    + "<br><b>" + tr("Card Art") + ":</b> " + tr("Image priority is decided in the following order")
    + "<ol><li>" + tr("The") + "<a href='https://github.com/Cockatrice/Cockatrice/wiki/Custom-Cards-%26-Sets#to-add-custom-art-for-cards-the-easiest-way-is-to-use-the-custom-folder'> "
    + tr("CUSTOM Folder") + "</a></li><li>" + tr("Enabled Sets (Top to Bottom)") + "</li><li>" + tr("Disabled Sets (Top to Bottom)") + "</li></ol>"
    );

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(actSave()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(actRestore()));

    mainLayout = new QGridLayout;
    mainLayout->addWidget(setsEditToolBar, 0, 0, 1, 1);
    mainLayout->addWidget(view, 0, 1, 1, 2);
    mainLayout->addWidget(enableAllButton, 1, 1);
    mainLayout->addWidget(disableAllButton, 1, 2);
    mainLayout->addWidget(enableSomeButton, 1, 1);
    mainLayout->addWidget(disableSomeButton, 1, 2);
    mainLayout->addWidget(labNotes, 2, 1, 1, 2);
    mainLayout->addWidget(buttonBox, 3, 1, 1, 2);
    mainLayout->setColumnStretch(1, 1);
    mainLayout->setColumnStretch(2, 1);

    enableSomeButton->hide();
    disableSomeButton->hide();

    QWidget *centralWidget = new QWidget;
    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);

    setWindowTitle(tr("Edit sets"));
    resize(700, 400);
}

WndSets::~WndSets()
{
}

void WndSets::rebuildMainLayout(int actionToTake)
{
    if (mainLayout == nullptr)
        return;

    switch (actionToTake)
    {
        case NO_SETS_SELECTED:
            enableAllButton->show();
            disableAllButton->show();
            enableSomeButton->hide();
            disableSomeButton->hide();
            break;

        case SOME_SETS_SELECTED:
            enableAllButton->hide();
            disableAllButton->hide();
            enableSomeButton->show();
            disableSomeButton->show();
            break;
    }
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

    int rows = view->selectionModel()->selectedRows().size();
    rebuildMainLayout((rows > 1) ? SOME_SETS_SELECTED : NO_SETS_SELECTED);

}

void WndSets::selectRows(QSet<int> rows)
{
    foreach (int i, rows)
    {
        QModelIndex idx = model->index(i, 0);
        view->selectionModel()->select(idx, QItemSelectionModel::Select | QItemSelectionModel::Rows);
        view->scrollTo(idx, QAbstractItemView::EnsureVisible);
    }
}


void WndSets::actEnableAll()
{
    model->toggleAll(true);

}

void WndSets::actDisableAll()
{
    model->toggleAll(false);
}

void WndSets::actEnableSome()
{
    QModelIndexList rows = view->selectionModel()->selectedRows();

    foreach(QModelIndex i, rows)
        model->toggleRow(i.row(), true);

}

void WndSets::actDisableSome()
{
    QModelIndexList rows = view->selectionModel()->selectedRows();

    foreach(QModelIndex i, rows)
        model->toggleRow(i.row(), false);
}

void WndSets::actUp()
{
    QModelIndexList rows = view->selectionModel()->selectedRows();
    qSort(rows.begin(), rows.end(), qLess<QModelIndex>());
    QSet<int> newRows;

    if (rows.empty())
        return;

    foreach (QModelIndex i, rows)
    {
        int oldRow = i.row();
        int newRow = oldRow - 1;
        if (oldRow <= 0)
            continue;

        model->swapRows(oldRow, newRow);
        newRows.insert(newRow);
    }

    selectRows(newRows);
}

void WndSets::actDown()
{
    QModelIndexList rows = view->selectionModel()->selectedRows();
    qSort(rows.begin(), rows.end(), qGreater<QModelIndex>());
    QSet<int> newRows;

    if (rows.empty())
        return;

    foreach (QModelIndex i, rows)
    {
        int oldRow = i.row();
        int newRow = oldRow + 1;
        if (oldRow >= model->rowCount() - 1)
            continue;

        model->swapRows(oldRow, newRow);
        newRows.insert(newRow);
    }

    selectRows(newRows);
}

void WndSets::actTop()
{
    QModelIndexList rows = view->selectionModel()->selectedRows();
    qSort(rows.begin(), rows.end(), qLess<QModelIndex>());
    QSet<int> newRows;
    int newRow = 0;

    if (rows.empty())
        return;

    for (int i = 0; i < rows.length(); i++)
    {
        int oldRow = rows.at(i).row();

        if (oldRow <= 0) {
            newRow++;
            continue;
        }

        newRows.insert(newRow);
        model->swapRows(oldRow, newRow++);
    }

    selectRows(newRows);
}

void WndSets::actBottom()
{
    QModelIndexList rows = view->selectionModel()->selectedRows();
    qSort(rows.begin(), rows.end(), qGreater<QModelIndex>());
    QSet<int> newRows;
    int newRow = model->rowCount() - 1;

    if (rows.empty())
        return;

    for (int i = 0; i < rows.length(); i++)
    {
        int oldRow = rows.at(i).row();

        if (oldRow >= newRow) {
            newRow--;
            continue;
        }

        newRows.insert(newRow);
        model->swapRows(oldRow, newRow--);
    }

    selectRows(newRows);
}
