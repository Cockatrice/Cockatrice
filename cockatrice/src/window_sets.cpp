#include "window_sets.h"
#include "setsmodel.h"
#include "main.h"
#include <QTreeView>
#include <QHBoxLayout>
#include <QHeaderView>

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

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addWidget(view);

    QWidget *centralWidget = new QWidget;
    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);

    setWindowTitle(tr("Edit sets"));
    resize(700, 400);
}

WndSets::~WndSets()
{
}
