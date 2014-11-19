#include "window_sets.h"
#include "setsmodel.h"
#include "main.h"
#include <QTreeView>
#include <QGridLayout>
#include <QHeaderView>
#include <QPushButton>

WndSets::WndSets(QWidget *parent)
    : QMainWindow(parent)
{
    model = new SetsModel(db, this);
    proxyModel = new SetsProxyModel(this);
    proxyModel->setSourceModel(model);
    proxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    view = new QTreeView;
    view->setModel(proxyModel);
    view->setAlternatingRowColors(true);
    view->setUniformRowHeights(true);
    view->setAllColumnsShowFocus(true);
    view->setSortingEnabled(true);
    view->sortByColumn(SetsModel::SortKeyCol, Qt::AscendingOrder);

    view->setDragEnabled(true);
    view->setAcceptDrops(true);
    view->setDropIndicatorShown(true);
    view->setDragDropMode(QAbstractItemView::InternalMove);
#if QT_VERSION < 0x050000
    view->header()->setResizeMode(SetsModel::LongNameCol, QHeaderView::ResizeToContents);
#else
    view->header()->setSectionResizeMode(SetsModel::LongNameCol, QHeaderView::ResizeToContents);
#endif

    saveButton = new QPushButton(tr("Save sets order"));
    connect(saveButton, SIGNAL(clicked()), this, SLOT(actSave()));
    restoreButton = new QPushButton(tr("Restore saved sets order"));
    connect(restoreButton, SIGNAL(clicked()), this, SLOT(actRestore()));

    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->addWidget(view, 0, 0, 1, 2);
    mainLayout->addWidget(saveButton, 1, 0, 1, 1);
    mainLayout->addWidget(restoreButton, 1, 1, 1, 1);

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
    proxyModel->saveOrder();
}

void WndSets::actRestore()
{
    view->sortByColumn(SetsModel::SortKeyCol, Qt::AscendingOrder);
}
