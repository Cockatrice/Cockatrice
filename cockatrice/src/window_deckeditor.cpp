#include <QtGui>
#include "window_deckeditor.h"
#include "carddatabase.h"
#include "carddatabasemodel.h"
#include "decklistmodel.h"
#include "cardinfowidget.h"

WndDeckEditor::WndDeckEditor(CardDatabase *_db, QWidget *parent)
	: QMainWindow(parent), db(_db)
{
	databaseModel = new CardDatabaseModel(db);
	databaseView = new QTreeView();
	databaseView->setModel(databaseModel);
	databaseView->setSortingEnabled(true);
	connect(databaseView->selectionModel(), SIGNAL(currentRowChanged(const QModelIndex &, const QModelIndex &)), this, SLOT(updateCardInfoLeft(const QModelIndex &, const QModelIndex &)));

	QVBoxLayout *leftFrame = new QVBoxLayout;
	leftFrame->addWidget(databaseView);

	cardInfo = new CardInfoWidget(db);
	cardInfo->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);

	QToolBar *verticalToolBar = new QToolBar;
	verticalToolBar->setOrientation(Qt::Vertical);
	QHBoxLayout *verticalToolBarLayout = new QHBoxLayout;
	verticalToolBarLayout->addStretch();
	verticalToolBarLayout->addWidget(verticalToolBar);
	verticalToolBarLayout->addStretch();

	QVBoxLayout *middleFrame = new QVBoxLayout;
	middleFrame->addWidget(cardInfo);
	middleFrame->addLayout(verticalToolBarLayout);
	middleFrame->addStretch();

	deckModel = new DeckListModel(db, this);
	deckView = new QTreeView();
	deckView->setModel(deckModel);
	connect(deckView->selectionModel(), SIGNAL(currentRowChanged(const QModelIndex &, const QModelIndex &)), this, SLOT(updateCardInfoRight(const QModelIndex &, const QModelIndex &)));

	QLabel *nameLabel = new QLabel(tr("Deck &name:"));
	nameEdit = new QLineEdit;
	nameLabel->setBuddy(nameEdit);
	connect(nameEdit, SIGNAL(textChanged(const QString &)), deckModel->getDeckList(), SLOT(setName(const QString &)));
	QLabel *commentsLabel = new QLabel(tr("&Comments:"));
	commentsEdit = new QLineEdit;
	commentsLabel->setBuddy(commentsEdit);
	connect(commentsEdit, SIGNAL(textChanged(const QString &)), deckModel->getDeckList(), SLOT(setComments(const QString &)));
	QGridLayout *grid = new QGridLayout;
	grid->addWidget(nameLabel, 0, 0);
	grid->addWidget(nameEdit, 0, 1);
	grid->addWidget(commentsLabel, 1, 0);
	grid->addWidget(commentsEdit, 1, 1);

	QVBoxLayout *rightFrame = new QVBoxLayout;
	rightFrame->addLayout(grid);
	rightFrame->addWidget(deckView);

	QHBoxLayout *mainLayout = new QHBoxLayout;
	mainLayout->addLayout(leftFrame, 10);
	mainLayout->addLayout(middleFrame);
	mainLayout->addLayout(rightFrame, 10);

	QWidget *centralWidget = new QWidget;
	centralWidget->setLayout(mainLayout);
	setCentralWidget(centralWidget);

	setWindowTitle(tr("Card database"));

	aNewDeck = new QAction(tr("&New deck"), this);
	connect(aNewDeck, SIGNAL(triggered()), this, SLOT(actNewDeck()));
	aLoadDeck = new QAction(tr("&Load deck..."), this);
	aLoadDeck->setShortcut(tr("Ctrl+L"));
	connect(aLoadDeck, SIGNAL(triggered()), this, SLOT(actLoadDeck()));
	aSaveDeck = new QAction(tr("&Save deck"), this);
	aSaveDeck->setShortcut(tr("Ctrl+S"));
	connect(aSaveDeck, SIGNAL(triggered()), this, SLOT(actSaveDeck()));
	aSaveDeckAs = new QAction(tr("&Save deck as..."), this);
	connect(aSaveDeckAs, SIGNAL(triggered()), this, SLOT(actSaveDeckAs()));

	deckMenu = menuBar()->addMenu(tr("&Deck"));
	deckMenu->addAction(aNewDeck);
	deckMenu->addAction(aLoadDeck);
	deckMenu->addAction(aSaveDeck);
	deckMenu->addAction(aSaveDeckAs);

	aAddCard = new QAction(tr("&Add card"), this);
	connect(aAddCard, SIGNAL(triggered()), this, SLOT(actAddCard()));
	aRemoveCard = new QAction(tr("&Remove card"), this);
	connect(aRemoveCard, SIGNAL(triggered()), this, SLOT(actRemoveCard()));
	aIncrement = new QAction(tr("&Increment number"), this);
	connect(aIncrement, SIGNAL(triggered()), this, SLOT(actIncrement()));
	aDecrement = new QAction(tr("&Decrement number"), this);
	connect(aDecrement, SIGNAL(triggered()), this, SLOT(actDecrement()));

	verticalToolBar->addAction(aAddCard);
	verticalToolBar->addAction(aRemoveCard);
	verticalToolBar->addAction(aIncrement);
	verticalToolBar->addAction(aDecrement);
}

WndDeckEditor::~WndDeckEditor()
{

}

void WndDeckEditor::updateCardInfoLeft(const QModelIndex &current, const QModelIndex &/*previous*/)
{
	cardInfo->setCard(current.sibling(current.row(), 0).data().toString());
}

void WndDeckEditor::updateCardInfoRight(const QModelIndex &current, const QModelIndex &/*previous*/)
{
	cardInfo->setCard(current.sibling(current.row(), 1).data().toString());
}

void WndDeckEditor::actNewDeck()
{
	deckModel->cleanList();
	lastFileName = QString();
}

void WndDeckEditor::actLoadDeck()
{
	DeckList *l = deckModel->getDeckList();
	if (l->loadDialog(this)) {
		lastFileName = l->getLastFileName();
		lastFileFormat = l->getLastFileFormat();
//		deckView->reset();
		nameEdit->setText(l->getName());
		commentsEdit->setText(l->getComments());
	}
}

void WndDeckEditor::actSaveDeck()
{
	if (lastFileName.isEmpty())
		actSaveDeckAs();
	else
		deckModel->getDeckList()->saveToFile(lastFileName, lastFileFormat);
;
}

void WndDeckEditor::actSaveDeckAs()
{
	DeckList *l = deckModel->getDeckList();
	if (l->saveDialog(this)) {
		lastFileName = l->getLastFileName();
		lastFileFormat = l->getLastFileFormat();
	}
}

void WndDeckEditor::actAddCard()
{
	const QModelIndex currentIndex = databaseView->selectionModel()->currentIndex();
	if (!currentIndex.isValid())
		return;
	const QString cardName = databaseModel->index(currentIndex.row(), 0).data().toString();
	QModelIndexList matches = deckModel->match(deckModel->index(0, 1), Qt::EditRole, cardName);
	if (matches.isEmpty()) {
		int row = deckModel->rowCount();
		deckModel->insertRow(row);
		deckModel->setData(deckModel->index(row, 1), cardName, Qt::EditRole);
	} else {
		const QModelIndex numberIndex = deckModel->index(matches[0].row(), 0);
		const int count = deckModel->data(numberIndex, Qt::EditRole).toInt();
		deckModel->setData(numberIndex, count + 1, Qt::EditRole);
	}
}

void WndDeckEditor::actRemoveCard()
{
	const QModelIndex currentIndex = deckView->selectionModel()->currentIndex();
	if (!currentIndex.isValid())
		return;
	deckModel->removeRow(currentIndex.row());
}

void WndDeckEditor::actIncrement()
{
	const QModelIndex currentIndex = deckView->selectionModel()->currentIndex();
	if (!currentIndex.isValid())
		return;
	const QModelIndex numberIndex = deckModel->index(currentIndex.row(), 0);
	const int count = deckModel->data(numberIndex, Qt::EditRole).toInt();
	deckModel->setData(numberIndex, count + 1, Qt::EditRole);
}

void WndDeckEditor::actDecrement()
{
	const QModelIndex currentIndex = deckView->selectionModel()->currentIndex();
	if (!currentIndex.isValid())
		return;
	const QModelIndex numberIndex = deckModel->index(currentIndex.row(), 0);
	const int count = deckModel->data(numberIndex, Qt::EditRole).toInt();
	if (count == 1)
		deckModel->removeRow(currentIndex.row());
	else
		deckModel->setData(numberIndex, count - 1, Qt::EditRole);
}
