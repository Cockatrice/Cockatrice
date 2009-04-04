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
	
	deckModel = new DeckListModel(db, this);
	deckView = new QTreeView();
	deckView->setModel(deckModel);
//	deckView->setSortingEnabled(true);
	connect(deckView->selectionModel(), SIGNAL(currentRowChanged(const QModelIndex &, const QModelIndex &)), this, SLOT(updateCardInfoRight(const QModelIndex &, const QModelIndex &)));
	
	cardInfo = new CardInfoWidget(db);
	
	QVBoxLayout *middleFrame = new QVBoxLayout;
	middleFrame->addWidget(cardInfo);
	middleFrame->addStretch();
	
	QHBoxLayout *mainLayout = new QHBoxLayout;
	mainLayout->addWidget(databaseView);
	mainLayout->addLayout(middleFrame);
	mainLayout->addWidget(deckView);
	
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
	
	deckMenu = menuBar()->addMenu(tr("Deck"));
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
	QString fileName = QFileDialog::getOpenFileName(this, tr("Load deck"), QString(), tr("Deck files (*.dec)"));
	if (fileName.isEmpty())
		return;
	
	if (deckModel->loadFromFile(fileName))
		lastFileName = fileName;
}

void WndDeckEditor::actSaveDeck()
{
	if (lastFileName.isEmpty())
		actSaveDeckAs();
	else
		deckModel->saveToFile(lastFileName);
}

void WndDeckEditor::actSaveDeckAs()
{
	QString fileName = QFileDialog::getSaveFileName(this, tr("Save deck as"), QString(), tr("Deck files (*.dec)"));
	if (!fileName.endsWith(".dec"))
		fileName.append(".dec");
	if (deckModel->saveToFile(fileName))
		lastFileName = fileName;
}
