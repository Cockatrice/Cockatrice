#include "window_sets.h"
#include "setsmodel.h"
#include <QtGui>

WndSets::WndSets(CardDatabase *_db, QWidget *parent)
	: QMainWindow(parent)
{
	model = new SetsModel(_db, this);
	view = new QTreeView;
	view->setModel(model);
	view->setAlternatingRowColors(true);
	view->setUniformRowHeights(true);
	view->setAllColumnsShowFocus(true);

	view->setDragEnabled(true);
	view->setAcceptDrops(true);
	view->setDropIndicatorShown(true);
	view->setDragDropMode(QAbstractItemView::InternalMove);

	QHBoxLayout *mainLayout = new QHBoxLayout;
	mainLayout->addWidget(view);

	QWidget *centralWidget = new QWidget;
	centralWidget->setLayout(mainLayout);
	setCentralWidget(centralWidget);

	setWindowTitle(tr("Edit sets"));
	resize(400, 400);
}

WndSets::~WndSets()
{
}
