#include "dlg_manage_sets.h"

#include "../interface/card_picture_loader/card_picture_loader.h"
#include "../interface/widgets/utility/custom_line_edit.h"
#include "../main.h"

#include <QAction>
#include <QCheckBox>
#include <QDebug>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QItemSelection>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QToolBar>
#include <QTreeView>
#include <algorithm>
#include <libcockatrice/card/database/card_database_manager.h>
#include <libcockatrice/models/database/card_set/card_sets_model.h>
#include <libcockatrice/settings/cache_settings.h>

#define SORT_RESET -1

WndSets::WndSets(QWidget *parent) : QMainWindow(parent)
{
    setOrderIsSorted = false;

    // left toolbar
    setsEditToolBar = new QToolBar;
    setsEditToolBar->setOrientation(Qt::Vertical);
    setsEditToolBar->setIconSize(QSize(24, 24));
    setsEditToolBar->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    aTop = new QAction(QString(), this);
    aTop->setIcon(QPixmap("theme:icons/arrow_top_green"));
    aTop->setToolTip(tr("Move selected set to the top"));
    aTop->setEnabled(false);
    connect(aTop, &QAction::triggered, this, &WndSets::actTop);
    setsEditToolBar->addAction(aTop);

    aUp = new QAction(QString(), this);
    aUp->setIcon(QPixmap("theme:icons/arrow_up_green"));
    aUp->setToolTip(tr("Move selected set up"));
    aUp->setEnabled(false);
    connect(aUp, &QAction::triggered, this, &WndSets::actUp);
    setsEditToolBar->addAction(aUp);

    aDown = new QAction(QString(), this);
    aDown->setIcon(QPixmap("theme:icons/arrow_down_green"));
    aDown->setToolTip(tr("Move selected set down"));
    aDown->setEnabled(false);
    connect(aDown, &QAction::triggered, this, &WndSets::actDown);
    setsEditToolBar->addAction(aDown);

    aBottom = new QAction(QString(), this);
    aBottom->setIcon(QPixmap("theme:icons/arrow_bottom_green"));
    aBottom->setToolTip(tr("Move selected set to the bottom"));
    aBottom->setEnabled(false);
    connect(aBottom, &QAction::triggered, this, &WndSets::actBottom);
    setsEditToolBar->addAction(aBottom);

    // search field
    searchField = new LineEditUnfocusable;
    searchField->setObjectName("searchEdit");
    searchField->setPlaceholderText(tr("Search by set name, code, or type"));
    searchField->addAction(QPixmap("theme:icons/search"), LineEditUnfocusable::LeadingPosition);
    searchField->setClearButtonEnabled(true);
    setFocusProxy(searchField);

    defaultSortButton = new QPushButton(tr("Default order"));
    defaultSortButton->setToolTip(tr("Restore original art priority order"));

    connect(defaultSortButton, &QPushButton::clicked, this, &WndSets::actRestoreOriginalOrder);

    filterBox = new QHBoxLayout;
    filterBox->addWidget(searchField);
    filterBox->addWidget(defaultSortButton);

    // view
    model = new SetsModel(CardDatabaseManager::getInstance(), this);
    displayModel = new SetsDisplayModel(this);
    displayModel->setSourceModel(model);
    displayModel->setDynamicSortFilter(false);
    view = new QTreeView;
    view->setModel(displayModel);

    view->setAlternatingRowColors(true);
    view->setUniformRowHeights(true);
    view->setAllColumnsShowFocus(true);
    view->setSortingEnabled(true);
    view->setSelectionBehavior(QAbstractItemView::SelectRows);
    view->setSelectionMode(QAbstractItemView::ExtendedSelection);

    view->setDragEnabled(true);
    view->setAcceptDrops(true);
    view->setDropIndicatorShown(true);
    view->setDragDropMode(QAbstractItemView::InternalMove);

    view->sortByColumn(SetsModel::SortKeyCol, Qt::AscendingOrder);
    view->setColumnHidden(SetsModel::SortKeyCol, true);
    view->setColumnHidden(SetsModel::IsKnownCol, true);
    view->setColumnHidden(SetsModel::PriorityCol, true);
    view->setRootIsDecorated(false);

    connect(view->header(), &QHeaderView::sectionClicked, this, &WndSets::actSort);

    // bottom buttons
    enableAllButton = new QPushButton(tr("Enable all sets"));
    disableAllButton = new QPushButton(tr("Disable all sets"));
    enableSomeButton = new QPushButton(tr("Enable selected set(s)"));
    disableSomeButton = new QPushButton(tr("Disable selected set(s)"));

    connect(enableAllButton, &QPushButton::clicked, this, &WndSets::actEnableAll);
    connect(disableAllButton, &QPushButton::clicked, this, &WndSets::actDisableAll);
    connect(enableSomeButton, &QPushButton::clicked, this, &WndSets::actEnableSome);
    connect(disableSomeButton, &QPushButton::clicked, this, &WndSets::actDisableSome);
    connect(view->selectionModel(), &QItemSelectionModel::selectionChanged, this, &WndSets::actToggleButtons);
    connect(searchField, &LineEditUnfocusable::textChanged, displayModel,
            qOverload<const QString &>(&SetsDisplayModel::setFilterRegularExpression));
    connect(view->header(), &QHeaderView::sortIndicatorChanged, this, &WndSets::actDisableSortButtons);
    connect(searchField, &LineEditUnfocusable::textChanged, this, &WndSets::actDisableResetButton);

    labNotes = new QLabel;
    labNotes->setWordWrap(true);
    labNotes->setTextInteractionFlags(Qt::TextBrowserInteraction);
    labNotes->setOpenExternalLinks(true);
    labNotes->setText(tr("Use CTRL+A to select all sets in the view.") + "<br><b>" + tr("Deck Editor") + ":</b> " +
                      tr("Only cards in enabled sets will appear in the card list of the deck editor.") + "<br><b>" +
                      tr("Card Art") + ":</b> " + tr("Image priority is decided in the following order:") + "<br>" +
                      tr("first the CUSTOM Folder (%1), then the Enabled Sets in this dialog (Top to Bottom)",
                         "%1 is a link to the wiki")
                          .arg("<a href='https://github.com/Cockatrice/Cockatrice/wiki/Custom-Cards-%26-Sets"
                               "#to-add-custom-art-for-cards-the-easiest-way-is-to-use-the-custom-folder'>" +
                               tr("How to use custom card art") + "</a>"));

    QGridLayout *hintsGrid = new QGridLayout;
    hintsGrid->addWidget(labNotes, 0, 0);
    hintsGroupBox = new QGroupBox(tr("Hints"));
    hintsGroupBox->setLayout(hintsGrid);

    sortWarning = new QGroupBox(tr("Note"));
    QGridLayout *sortWarningLayout = new QGridLayout;
    sortWarningText = new QLabel;
    sortWarningText->setWordWrap(true);
    sortWarningText->setText(tr("Sorting by column allows you to find a set while not changing set priority.") + " " +
                             tr("To enable ordering again, click the column header until this message disappears."));
    sortWarningLayout->addWidget(sortWarningText, 0, 0, 1, 2);
    sortWarningButton = new QPushButton;
    sortWarningButton->setText(tr("Use the current sorting as the set priority instead"));
    sortWarningButton->setToolTip(tr("Sorts the set priority using the same column"));
    sortWarningButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(sortWarningButton, &QPushButton::released, this, &WndSets::actIgnoreWarning);
    sortWarningLayout->addWidget(sortWarningButton, 1, 0);
    sortWarning->setLayout(sortWarningLayout);
    sortWarning->setVisible(false);

    includeRebalancedCards = SettingsCache::instance().getIncludeRebalancedCards();
    QCheckBox *includeRebalancedCardsCheckBox =
        new QCheckBox(tr("Include cards rebalanced for Alchemy [requires restart]"));
    includeRebalancedCardsCheckBox->setChecked(includeRebalancedCards);
    connect(includeRebalancedCardsCheckBox, &QAbstractButton::toggled, this, &WndSets::includeRebalancedCardsChanged);

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &WndSets::actSave);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &WndSets::actRestore);

    mainLayout = new QGridLayout;
    mainLayout->addLayout(filterBox, 0, 1, 1, 2);
    mainLayout->addWidget(setsEditToolBar, 1, 0, 4, 1);
    mainLayout->addWidget(view, 1, 1, 1, 2);
    mainLayout->addWidget(enableAllButton, 2, 1);
    mainLayout->addWidget(disableAllButton, 2, 2);
    mainLayout->addWidget(enableSomeButton, 2, 1);
    mainLayout->addWidget(disableSomeButton, 2, 2);
    mainLayout->addWidget(sortWarning, 3, 1, 1, 2);
    mainLayout->addWidget(includeRebalancedCardsCheckBox, 4, 1, 1, 2);
    mainLayout->addWidget(hintsGroupBox, 5, 1, 1, 2);
    mainLayout->addWidget(buttonBox, 6, 1, 1, 2);
    mainLayout->setColumnStretch(1, 1);
    mainLayout->setColumnStretch(2, 1);

    enableSomeButton->hide();
    disableSomeButton->hide();

    QWidget *centralWidget = new QWidget;
    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);

    setWindowTitle(tr("Manage sets"));
    setMinimumSize(800, 500);
    auto &geometry = SettingsCache::instance().getSetsDialogGeometry();
    if (!geometry.isEmpty()) {
        restoreGeometry(geometry);
    }
    auto &headerState = SettingsCache::instance().layouts().getSetsDialogHeaderState();
    if (!headerState.isEmpty()) {
        view->header()->restoreState(headerState);
        view->header()->setSortIndicator(SORT_RESET, Qt::DescendingOrder);
    } else {
        view->header()->resizeSections(QHeaderView::ResizeToContents);
    }
    connect(view->header(), &QHeaderView::geometriesChanged, this, &WndSets::saveHeaderState);
}

WndSets::~WndSets()
{
}

void WndSets::closeEvent(QCloseEvent * /*ev*/)
{
    SettingsCache::instance().setSetsDialogGeometry(saveGeometry());
}

void WndSets::saveHeaderState()
{
    SettingsCache::instance().layouts().setSetsDialogHeaderState(view->header()->saveState());
}

void WndSets::rebuildMainLayout(int actionToTake)
{
    if (mainLayout == nullptr)
        return;

    switch (actionToTake) {
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

void WndSets::includeRebalancedCardsChanged(bool _includeRebalancedCards)
{
    includeRebalancedCards = _includeRebalancedCards;
}

void WndSets::actSave()
{
    model->save(CardDatabaseManager::getInstance());
    SettingsCache::instance().setIncludeRebalancedCards(includeRebalancedCards);
    CardPictureLoader::clearPixmapCache();
    close();
}

void WndSets::actRestore()
{
    model->restore(CardDatabaseManager::getInstance());
    close();
}

void WndSets::actRestoreOriginalOrder()
{
    view->header()->setSortIndicator(SORT_RESET, Qt::DescendingOrder);
    model->restoreOriginalOrder();
    sortWarning->setVisible(false);
}

void WndSets::actDisableResetButton(const QString &filterString)
{
    if (filterString.isEmpty()) {
        defaultSortButton->setEnabled(true);
    } else {
        defaultSortButton->setEnabled(false);
    }
}

void WndSets::actSort(int index)
{
    if (sortIndex != index) {
        view->sortByColumn(index, Qt::AscendingOrder);
        sortOrder = Qt::AscendingOrder;
        sortIndex = index;
        sortWarning->setVisible(true);
    } else {
        if (sortOrder == Qt::AscendingOrder) {
            view->sortByColumn(index, Qt::DescendingOrder);
            sortOrder = Qt::DescendingOrder;
            sortIndex = index;
            sortWarning->setVisible(true);
        } else {
            view->header()->setSortIndicator(SORT_RESET, Qt::DescendingOrder);
            sortIndex = -1;
            sortWarning->setVisible(false);
        }
    }
}

void WndSets::actIgnoreWarning()
{
    if (sortIndex < 0) {
        return;
    }
    model->sort(sortIndex, sortOrder);
    view->header()->setSortIndicator(SORT_RESET, Qt::DescendingOrder);
    sortIndex = -1;
    sortWarning->setVisible(false);
}

void WndSets::actDisableSortButtons(int index)
{
    if (index != SORT_RESET) {
        view->setDragEnabled(false);
        setOrderIsSorted = true;
    } else {
        setOrderIsSorted = false;
        view->setDragEnabled(true);
    }
    if (!view->selectionModel()->selection().empty()) {
        view->scrollTo(view->selectionModel()->selectedRows().first());
    }
    actToggleButtons(view->selectionModel()->selection(), QItemSelection());
}

void WndSets::actToggleButtons(const QItemSelection &selected, const QItemSelection &)
{
    bool emptySelection = selected.empty();
    aTop->setDisabled(emptySelection || setOrderIsSorted);
    aUp->setDisabled(emptySelection || setOrderIsSorted);
    aDown->setDisabled(emptySelection || setOrderIsSorted);
    aBottom->setDisabled(emptySelection || setOrderIsSorted);

    int rows = view->selectionModel()->selectedRows().size();
    rebuildMainLayout((rows > 1) ? SOME_SETS_SELECTED : NO_SETS_SELECTED);
}

void WndSets::selectRows(QSet<int> rows)
{
    for (auto i : rows) {
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

    for (auto i : rows) {
        model->toggleRow(displayModel->mapToSource(i).row(), true);
    }
}

void WndSets::actDisableSome()
{
    QModelIndexList rows = view->selectionModel()->selectedRows();

    for (auto i : rows) {
        model->toggleRow(displayModel->mapToSource(i).row(), false);
    }
}

void WndSets::actUp()
{
    QModelIndexList rows = view->selectionModel()->selectedRows();
    std::sort(rows.begin(), rows.end(), std::less<QModelIndex>());
    QSet<int> newRows;

    if (rows.empty())
        return;

    for (auto i : rows) {
        if (i.row() <= 0)
            continue;
        int oldRow = displayModel->mapToSource(i).row();
        int newRow = i.row() - 1;

        model->swapRows(oldRow, displayModel->mapToSource(displayModel->index(newRow, 0)).row());
        newRows.insert(newRow);
    }

    selectRows(newRows);
}

void WndSets::actDown()
{
    QModelIndexList rows = view->selectionModel()->selectedRows();
    // QModelIndex only implements operator<, so we can't use std::greater
    std::sort(rows.begin(), rows.end(), [](const QModelIndex &a, const QModelIndex &b) { return b < a; });
    QSet<int> newRows;

    if (rows.empty())
        return;

    for (auto i : rows) {
        if (i.row() >= displayModel->rowCount() - 1)
            continue;
        int oldRow = displayModel->mapToSource(i).row();
        int newRow = i.row() + 1;

        model->swapRows(oldRow, displayModel->mapToSource(displayModel->index(newRow, 0)).row());
        newRows.insert(newRow);
    }

    selectRows(newRows);
}

void WndSets::actTop()
{
    QModelIndexList rows = view->selectionModel()->selectedRows();
    std::sort(rows.begin(), rows.end(), std::less<QModelIndex>());
    QSet<int> newRows;
    int newRow = 0;

    if (rows.empty())
        return;

    for (int i = 0; i < rows.length(); i++) {
        int oldRow = displayModel->mapToSource(rows.at(i)).row();

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
    // QModelIndex only implements operator<, so we can't use std::greater
    std::sort(rows.begin(), rows.end(), [](const QModelIndex &a, const QModelIndex &b) { return b < a; });
    QSet<int> newRows;
    int newRow = model->rowCount() - 1;

    if (rows.empty())
        return;

    for (int i = 0; i < rows.length(); i++) {
        int oldRow = displayModel->mapToSource(rows.at(i)).row();

        if (oldRow >= newRow) {
            newRow--;
            continue;
        }

        newRows.insert(newRow);
        model->swapRows(oldRow, newRow--);
    }

    selectRows(newRows);
}
