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

	QVBoxLayout *middleFrame = new QVBoxLayout;
	middleFrame->addWidget(cardInfo);
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
	mainLayout->addLayout(leftFrame);
	mainLayout->addLayout(middleFrame);
	mainLayout->addLayout(rightFrame);

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
}

WndDeckEditor::~WndDeckEditor()
{

}

void WndDeckEditor::updateCardInfoLeft(const QModelIndex &current, const QModelIndex &previous)
{
	Q_UNUSED(previous);
	cardInfo->setCard(current.sibling(current.row(), 0).data().toString());
}

void WndDeckEditor::updateCardInfoRight(const QModelIndex &current, const QModelIndex &previous)
{
	Q_UNUSED(previous);
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
		deckView->reset();
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
