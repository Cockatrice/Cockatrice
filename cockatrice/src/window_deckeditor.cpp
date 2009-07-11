#include <QtGui>
#include "window_deckeditor.h"
#include "window_sets.h"
#include "carddatabase.h"
#include "carddatabasemodel.h"
#include "decklistmodel.h"
#include "cardinfowidget.h"

WndDeckEditor::WndDeckEditor(CardDatabase *_db, QWidget *parent)
	: QMainWindow(parent), db(_db)
{
	QLabel *searchLabel = new QLabel(tr("&Search for:"));
	searchEdit = new QLineEdit;
	searchLabel->setBuddy(searchEdit);
	connect(searchEdit, SIGNAL(textChanged(const QString &)), this, SLOT(updateSearch(const QString &)));
	connect(searchEdit, SIGNAL(returnPressed()), this, SLOT(actAddCard()));

	QHBoxLayout *searchLayout = new QHBoxLayout;
	searchLayout->addWidget(searchLabel);
	searchLayout->addWidget(searchEdit);

	databaseModel = new CardDatabaseModel(db);
	databaseView = new QTreeView();
	databaseView->setModel(databaseModel);
	databaseView->setUniformRowHeights(true);
	databaseView->setSortingEnabled(true);
	databaseView->resizeColumnToContents(0);
	connect(databaseView->selectionModel(), SIGNAL(currentRowChanged(const QModelIndex &, const QModelIndex &)), this, SLOT(updateCardInfoLeft(const QModelIndex &, const QModelIndex &)));
	connect(databaseView, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(actAddCard()));

	QVBoxLayout *leftFrame = new QVBoxLayout;
	leftFrame->addLayout(searchLayout);
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
	deckView->setUniformRowHeights(true);
	connect(deckView->selectionModel(), SIGNAL(currentRowChanged(const QModelIndex &, const QModelIndex &)), this, SLOT(updateCardInfoRight(const QModelIndex &, const QModelIndex &)));

	QLabel *nameLabel = new QLabel(tr("Deck &name:"));
	nameEdit = new QLineEdit;
	nameLabel->setBuddy(nameEdit);
	connect(nameEdit, SIGNAL(textChanged(const QString &)), this, SLOT(updateName(const QString &)));
	QLabel *commentsLabel = new QLabel(tr("&Comments:"));
	commentsEdit = new QTextEdit;
	commentsEdit->setMaximumHeight(70);
	commentsLabel->setBuddy(commentsEdit);
	connect(commentsEdit, SIGNAL(textChanged()), this, SLOT(updateComments()));
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

	setWindowTitle(tr("Deck editor [*]"));

	aNewDeck = new QAction(tr("&New deck"), this);
	aNewDeck->setShortcuts(QKeySequence::New);
	connect(aNewDeck, SIGNAL(triggered()), this, SLOT(actNewDeck()));
	aLoadDeck = new QAction(tr("&Load deck..."), this);
	aLoadDeck->setShortcuts(QKeySequence::Open);
	connect(aLoadDeck, SIGNAL(triggered()), this, SLOT(actLoadDeck()));
	aSaveDeck = new QAction(tr("&Save deck"), this);
	aSaveDeck->setShortcuts(QKeySequence::Save);
	connect(aSaveDeck, SIGNAL(triggered()), this, SLOT(actSaveDeck()));
	aSaveDeckAs = new QAction(tr("&Save deck as..."), this);
//	aSaveDeckAs->setShortcuts(QKeySequence::SaveAs);
	connect(aSaveDeckAs, SIGNAL(triggered()), this, SLOT(actSaveDeckAs()));
	aPrintDeck = new QAction(tr("&Print deck..."), this);
	aPrintDeck->setShortcuts(QKeySequence::Print);
	connect(aPrintDeck, SIGNAL(triggered()), this, SLOT(actPrintDeck()));
	aClose = new QAction(tr("&Close"), this);
	aClose->setShortcut(tr("Ctrl+Q"));
	connect(aClose, SIGNAL(triggered()), this, SLOT(close()));

	aEditSets = new QAction(tr("&Edit sets..."), this);
	connect(aEditSets, SIGNAL(triggered()), this, SLOT(actEditSets()));

	deckMenu = menuBar()->addMenu(tr("&Deck"));
	deckMenu->addAction(aNewDeck);
	deckMenu->addAction(aLoadDeck);
	deckMenu->addAction(aSaveDeck);
	deckMenu->addAction(aSaveDeckAs);
	deckMenu->addSeparator();
	deckMenu->addAction(aPrintDeck);
	deckMenu->addSeparator();
	deckMenu->addAction(aClose);

	setsMenu = menuBar()->addMenu(tr("&Sets"));
	setsMenu->addAction(aEditSets);

	aAddCard = new QAction(tr("Add card to &maindeck"), this);
	connect(aAddCard, SIGNAL(triggered()), this, SLOT(actAddCard()));
	aAddCard->setShortcut(tr("Ctrl+M"));
	aAddCardToSideboard = new QAction(tr("Add card to &sideboard"), this);
	connect(aAddCardToSideboard, SIGNAL(triggered()), this, SLOT(actAddCardToSideboard()));
	aAddCardToSideboard->setShortcut(tr("Ctrl+N"));
	aRemoveCard = new QAction(tr("&Remove row"), this);
	connect(aRemoveCard, SIGNAL(triggered()), this, SLOT(actRemoveCard()));
	aRemoveCard->setShortcut(tr("Del"));
	aIncrement = new QAction(tr("&Increment number"), this);
	connect(aIncrement, SIGNAL(triggered()), this, SLOT(actIncrement()));
	aIncrement->setShortcut(tr("+"));
	aDecrement = new QAction(tr("&Decrement number"), this);
	connect(aDecrement, SIGNAL(triggered()), this, SLOT(actDecrement()));
	aDecrement->setShortcut(tr("-"));

	verticalToolBar->addAction(aAddCard);
	verticalToolBar->addAction(aAddCardToSideboard);
	verticalToolBar->addAction(aRemoveCard);
	verticalToolBar->addAction(aIncrement);
	verticalToolBar->addAction(aDecrement);
}

WndDeckEditor::~WndDeckEditor()
{

}

void WndDeckEditor::updateName(const QString &name)
{
	deckModel->getDeckList()->setName(name);
	setWindowModified(true);
}

void WndDeckEditor::updateComments()
{
	deckModel->getDeckList()->setComments(commentsEdit->toPlainText());
	setWindowModified(true);
}

void WndDeckEditor::updateCardInfoLeft(const QModelIndex &current, const QModelIndex &/*previous*/)
{
	cardInfo->setCard(current.sibling(current.row(), 0).data().toString());
}

void WndDeckEditor::updateCardInfoRight(const QModelIndex &current, const QModelIndex &/*previous*/)
{
	if (!current.isValid())
		return;
	if (!current.model()->hasChildren(current.sibling(current.row(), 0)))
		cardInfo->setCard(current.sibling(current.row(), 1).data().toString());
}

void WndDeckEditor::updateSearch(const QString &search)
{
	QModelIndexList matches = databaseModel->match(databaseModel->index(0, 0), Qt::DisplayRole, search);
	if (matches.isEmpty())
		return;
	databaseView->selectionModel()->setCurrentIndex(matches[0], QItemSelectionModel::SelectCurrent);
}

bool WndDeckEditor::confirmClose()
{
	if (isWindowModified()) {
		QMessageBox::StandardButton ret = QMessageBox::warning(this, tr("Are you sure?"),
			tr("The decklist has been modified.\nDo you want to save the changes?"),
			QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
		if (ret == QMessageBox::Save)
			return actSaveDeck();
		else if (ret == QMessageBox::Cancel)
			return false;
	}
	return true;
}

void WndDeckEditor::closeEvent(QCloseEvent *event)
{
	if (confirmClose())
		event->accept();
	else
		event->ignore();
}

void WndDeckEditor::actNewDeck()
{
	if (!confirmClose())
		return;

	deckModel->cleanList();
	nameEdit->setText(QString());
	commentsEdit->setText(QString());
	lastFileName = QString();
}

void WndDeckEditor::actLoadDeck()
{
	if (!confirmClose())
		return;

	DeckList *l = deckModel->getDeckList();
	if (l->loadDialog(this)) {
		lastFileName = l->getLastFileName();
		lastFileFormat = l->getLastFileFormat();
		nameEdit->setText(l->getName());
		commentsEdit->setText(l->getComments());
		deckModel->sort(1);
		deckView->expandAll();
		deckView->resizeColumnToContents(0);
		setWindowModified(false);
	}
}

bool WndDeckEditor::actSaveDeck()
{
	if (lastFileName.isEmpty())
		return actSaveDeckAs();
	else if (deckModel->getDeckList()->saveToFile(lastFileName, lastFileFormat)) {
		setWindowModified(false);
		return true;
	} else
		return false;
}

bool WndDeckEditor::actSaveDeckAs()
{
	DeckList *l = deckModel->getDeckList();
	if (l->saveDialog(this)) {
		lastFileName = l->getLastFileName();
		lastFileFormat = l->getLastFileFormat();
		setWindowModified(false);
		return true;
	} else
		return false;
}

void WndDeckEditor::actPrintDeck()
{
	QPrintPreviewDialog *dlg = new QPrintPreviewDialog(this);
	connect(dlg, SIGNAL(paintRequested(QPrinter *)), deckModel, SLOT(printDeckList(QPrinter *)));
	dlg->exec();
}

void WndDeckEditor::actEditSets()
{
	WndSets *w = new WndSets(db, this);
	w->setWindowModality(Qt::WindowModal);
	w->show();
}

void WndDeckEditor::recursiveExpand(const QModelIndex &index)
{
	if (index.parent().isValid())
		recursiveExpand(index.parent());
	deckView->expand(index);
}

void WndDeckEditor::addCardHelper(const QString &zoneName)
{
	const QModelIndex currentIndex = databaseView->selectionModel()->currentIndex();
	if (!currentIndex.isValid())
		return;
	const QString cardName = databaseModel->index(currentIndex.row(), 0).data().toString();

	QModelIndex newCardIndex = deckModel->addCard(cardName, zoneName);
	recursiveExpand(newCardIndex);
	deckView->setCurrentIndex(newCardIndex);

	setWindowModified(true);
}

void WndDeckEditor::actAddCard()
{
	addCardHelper("main");
}

void WndDeckEditor::actAddCardToSideboard()
{
	addCardHelper("side");
}

void WndDeckEditor::actRemoveCard()
{
	const QModelIndex &currentIndex = deckView->selectionModel()->currentIndex();
	if (!currentIndex.isValid() || deckModel->hasChildren(currentIndex))
		return;
	deckModel->removeRow(currentIndex.row(), currentIndex.parent());
	setWindowModified(true);
}

void WndDeckEditor::actIncrement()
{
	const QModelIndex &currentIndex = deckView->selectionModel()->currentIndex();
	if (!currentIndex.isValid())
		return;
	const QModelIndex numberIndex = currentIndex.sibling(currentIndex.row(), 0);
	const int count = deckModel->data(numberIndex, Qt::EditRole).toInt();
	deckView->setCurrentIndex(numberIndex);
	deckModel->setData(numberIndex, count + 1, Qt::EditRole);
	setWindowModified(true);
}

void WndDeckEditor::actDecrement()
{
	const QModelIndex &currentIndex = deckView->selectionModel()->currentIndex();
	if (!currentIndex.isValid())
		return;
	const QModelIndex numberIndex = currentIndex.sibling(currentIndex.row(), 0);
	const int count = deckModel->data(numberIndex, Qt::EditRole).toInt();
	deckView->setCurrentIndex(numberIndex);
	if (count == 1)
		deckModel->removeRow(currentIndex.row(), currentIndex.parent());
	else
		deckModel->setData(numberIndex, count - 1, Qt::EditRole);
	setWindowModified(true);
}
