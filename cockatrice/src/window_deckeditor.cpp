#include <QLabel>
#include <QToolButton>
#include <QTreeView>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QToolBar>
#include <QTextEdit>
#include <QMenu>
#include <QAction>
#include <QCloseEvent>
#include <QFileDialog>
#include <QMenuBar>
#include <QMessageBox>
#include <QPrintPreviewDialog>
#include <QHeaderView>
#include <QApplication>
#include <QClipboard>
#include <QTextStream>
#include "window_deckeditor.h"
#include "window_sets.h"
#include "carddatabase.h"
#include "carddatabasemodel.h"
#include "decklistmodel.h"
#include "cardinfowidget.h"
#include "dlg_cardsearch.h"
#include "dlg_load_deck_from_clipboard.h"
#include "main.h"
#include "settingscache.h"
#include "priceupdater.h"

void SearchLineEdit::keyPressEvent(QKeyEvent *event)
{
	if (treeView && ((event->key() == Qt::Key_Up) || (event->key() == Qt::Key_Down)))
		QCoreApplication::sendEvent(treeView, event);
	QLineEdit::keyPressEvent(event);
}

WndDeckEditor::WndDeckEditor(QWidget *parent)
	: QMainWindow(parent)
{
	setAttribute(Qt::WA_DeleteOnClose);
	
	aSearch = new QAction(tr("&Search..."), this);
	aSearch->setIcon(QIcon(":/resources/icon_search.svg"));
	connect(aSearch, SIGNAL(triggered()), this, SLOT(actSearch()));
	aClearSearch = new QAction(tr("&Clear search"), this);
	aClearSearch->setIcon(QIcon(":/resources/icon_clearsearch.svg"));
	connect(aClearSearch, SIGNAL(triggered()), this, SLOT(actClearSearch()));

	QLabel *searchLabel = new QLabel(tr("&Search for:"));
	searchEdit = new SearchLineEdit;
	searchLabel->setBuddy(searchEdit);
	connect(searchEdit, SIGNAL(textChanged(const QString &)), this, SLOT(updateSearch(const QString &)));
	connect(searchEdit, SIGNAL(returnPressed()), this, SLOT(actAddCard()));
	QToolButton *searchButton = new QToolButton;
	searchButton->setDefaultAction(aSearch);
	QToolButton *clearSearchButton = new QToolButton;
	clearSearchButton->setDefaultAction(aClearSearch);

	QHBoxLayout *searchLayout = new QHBoxLayout;
	searchLayout->addWidget(searchLabel);
	searchLayout->addWidget(searchEdit);
	searchLayout->addWidget(searchButton);
	searchLayout->addWidget(clearSearchButton);

	databaseModel = new CardDatabaseModel(db, this);
	databaseDisplayModel = new CardDatabaseDisplayModel(this);
	databaseDisplayModel->setSourceModel(databaseModel);
	databaseDisplayModel->setFilterKeyColumn(0);
	databaseDisplayModel->sort(0, Qt::AscendingOrder);
	databaseView = new QTreeView();
	databaseView->setModel(databaseDisplayModel);
	databaseView->setUniformRowHeights(true);
	databaseView->setRootIsDecorated(false);
	databaseView->setAlternatingRowColors(true);
	databaseView->setSortingEnabled(true);
	databaseView->sortByColumn(0, Qt::AscendingOrder);
	databaseView->resizeColumnToContents(0);
	connect(databaseView->selectionModel(), SIGNAL(currentRowChanged(const QModelIndex &, const QModelIndex &)), this, SLOT(updateCardInfoLeft(const QModelIndex &, const QModelIndex &)));
	connect(databaseView, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(actAddCard()));
	searchEdit->setTreeView(databaseView);

	QVBoxLayout *leftFrame = new QVBoxLayout;
	leftFrame->addLayout(searchLayout);
	leftFrame->addWidget(databaseView);

	cardInfo = new CardInfoWidget(CardInfoWidget::ModeDeckEditor);
	cardInfo->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);

	QToolBar *verticalToolBar = new QToolBar;
	verticalToolBar->setOrientation(Qt::Vertical);
	verticalToolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	verticalToolBar->setIconSize(QSize(24, 24));
	QHBoxLayout *verticalToolBarLayout = new QHBoxLayout;
	verticalToolBarLayout->addStretch();
	verticalToolBarLayout->addWidget(verticalToolBar);
	verticalToolBarLayout->addStretch();

	QVBoxLayout *middleFrame = new QVBoxLayout;
	middleFrame->addWidget(cardInfo, 10);
	middleFrame->addLayout(verticalToolBarLayout);

	deckModel = new DeckListModel(this);
	connect(deckModel, SIGNAL(deckHashChanged()), this, SLOT(updateHash()));
	deckView = new QTreeView();
	deckView->setModel(deckModel);
	deckView->setUniformRowHeights(true);
	deckView->header()->setResizeMode(QHeaderView::ResizeToContents);
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
	QLabel *hashLabel1 = new QLabel(tr("Hash:"));
	hashLabel = new QLabel;

	QGridLayout *grid = new QGridLayout;
	grid->addWidget(nameLabel, 0, 0);
	grid->addWidget(nameEdit, 0, 1);

	grid->addWidget(commentsLabel, 1, 0);
	grid->addWidget(commentsEdit, 1, 1);
	
	grid->addWidget(hashLabel1, 2, 0);
	grid->addWidget(hashLabel, 2, 1);

        // Update price
        aUpdatePrices = new QAction(tr("&Update prices"), this);
        aUpdatePrices->setShortcut(tr("Ctrl+U"));
        aUpdatePrices->setIcon(QIcon(":/resources/icon_update.png"));
        connect(aUpdatePrices, SIGNAL(triggered()), this, SLOT(actUpdatePrices()));
	if (!settingsCache->getPriceTagFeature())
		aUpdatePrices->setVisible(false);
	
        QToolBar *deckToolBar = new QToolBar;
        deckToolBar->setOrientation(Qt::Vertical);
        deckToolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        deckToolBar->setIconSize(QSize(24, 24));
        deckToolBar->addAction(aUpdatePrices);
        QHBoxLayout *deckToolbarLayout = new QHBoxLayout;
        deckToolbarLayout->addStretch();
        deckToolbarLayout->addWidget(deckToolBar);
        deckToolbarLayout->addStretch();
	
	QVBoxLayout *rightFrame = new QVBoxLayout;
	rightFrame->addLayout(grid);
	rightFrame->addWidget(deckView, 10);
        rightFrame->addLayout(deckToolbarLayout);

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
	aSaveDeckAs = new QAction(tr("Save deck &as..."), this);
//	aSaveDeckAs->setShortcuts(QKeySequence::SaveAs);
	connect(aSaveDeckAs, SIGNAL(triggered()), this, SLOT(actSaveDeckAs()));
	aLoadDeckFromClipboard = new QAction(tr("Load deck from cl&ipboard..."), this);
	connect(aLoadDeckFromClipboard, SIGNAL(triggered()), this, SLOT(actLoadDeckFromClipboard()));
	aLoadDeckFromClipboard->setShortcuts(QKeySequence::Paste);
	aSaveDeckToClipboard = new QAction(tr("Save deck to clip&board"), this);
	connect(aSaveDeckToClipboard, SIGNAL(triggered()), this, SLOT(actSaveDeckToClipboard()));
	aSaveDeckToClipboard->setShortcuts(QKeySequence::Copy);
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
	deckMenu->addAction(aLoadDeckFromClipboard);
	deckMenu->addAction(aSaveDeckToClipboard);
	deckMenu->addSeparator();
	deckMenu->addAction(aPrintDeck);
	deckMenu->addSeparator();
	deckMenu->addAction(aClose);

	dbMenu = menuBar()->addMenu(tr("&Card database"));
	dbMenu->addAction(aEditSets);
	dbMenu->addSeparator();
	dbMenu->addAction(aSearch);
	dbMenu->addAction(aClearSearch);

	aAddCard = new QAction(tr("Add card to &maindeck"), this);
	aAddCard->setShortcuts(QList<QKeySequence>() << QKeySequence(tr("Return")) << QKeySequence(tr("Enter")));
	aAddCard->setIcon(QIcon(":/resources/arrow_right_green.svg"));
	connect(aAddCard, SIGNAL(triggered()), this, SLOT(actAddCard()));
	aAddCardToSideboard = new QAction(tr("Add card to &sideboard"), this);
        aAddCardToSideboard->setIcon(QIcon(":/resources/add_to_sideboard.svg"));
	aAddCardToSideboard->setShortcuts(QList<QKeySequence>() << QKeySequence(tr("Ctrl+Return")) << QKeySequence(tr("Ctrl+Enter")));
	connect(aAddCardToSideboard, SIGNAL(triggered()), this, SLOT(actAddCardToSideboard()));
	aRemoveCard = new QAction(tr("&Remove row"), this);
	aRemoveCard->setShortcut(tr("Del"));
        aRemoveCard->setIcon(QIcon(":/resources/remove_row.svg"));
	connect(aRemoveCard, SIGNAL(triggered()), this, SLOT(actRemoveCard()));
	aIncrement = new QAction(tr("&Increment number"), this);
	aIncrement->setShortcut(tr("+"));
        aIncrement->setIcon(QIcon(":/resources/increment.svg"));
	connect(aIncrement, SIGNAL(triggered()), this, SLOT(actIncrement()));
	aDecrement = new QAction(tr("&Decrement number"), this);
        aDecrement->setIcon(QIcon(":/resources/decrement.svg"));
	aDecrement->setShortcut(tr("-"));
	connect(aDecrement, SIGNAL(triggered()), this, SLOT(actDecrement()));

	verticalToolBar->addAction(aAddCard);
	verticalToolBar->addAction(aAddCardToSideboard);
	verticalToolBar->addAction(aRemoveCard);
	verticalToolBar->addAction(aIncrement);
	verticalToolBar->addAction(aDecrement);
	verticalToolBar->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
	
	dlgCardSearch = new DlgCardSearch(this);
	
	resize(950, 700);
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
	databaseDisplayModel->setCardNameBeginning(search);
	QModelIndexList sel = databaseView->selectionModel()->selectedRows();
	if (sel.isEmpty() && databaseDisplayModel->rowCount())
		databaseView->selectionModel()->setCurrentIndex(databaseDisplayModel->index(0, 0), QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
}

void WndDeckEditor::updateHash()
{
	hashLabel->setText(deckModel->getDeckList()->getDeckHash());
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
	if (confirmClose()) {
		event->accept();
		deleteLater();
	} else
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
	setWindowModified(false);
}

void WndDeckEditor::actLoadDeck()
{
	if (!confirmClose())
		return;

	QFileDialog dialog(this, tr("Load deck"));
	dialog.setDirectory(settingsCache->getDeckPath());
	dialog.setNameFilters(DeckList::fileNameFilters);
	if (!dialog.exec())
		return;

	QString fileName = dialog.selectedFiles().at(0);
	DeckList::FileFormat fmt = DeckList::getFormatFromNameFilter(dialog.selectedNameFilter());
	DeckList *l = new DeckList;
	if (l->loadFromFile(fileName, fmt))
		setDeck(l, fileName, fmt);
	else
		delete l;
}

bool WndDeckEditor::actSaveDeck()
{
	if (lastFileName.isEmpty())
		return actSaveDeckAs();
	else if (deckModel->getDeckList()->saveToFile(lastFileName, lastFileFormat)) {
		setWindowModified(false);
		return true;
	}
	QMessageBox::critical(this, tr("Error"), tr("The deck could not be saved.\nPlease check that the directory is writable and try again."));
	return false;
}

bool WndDeckEditor::actSaveDeckAs()
{
	QFileDialog dialog(this, tr("Save deck"));
	dialog.setDirectory(settingsCache->getDeckPath());
	dialog.setAcceptMode(QFileDialog::AcceptSave);
	dialog.setConfirmOverwrite(true);
	dialog.setDefaultSuffix("cod");
	dialog.setNameFilters(DeckList::fileNameFilters);
	dialog.selectFile(deckModel->getDeckList()->getName());
	if (!dialog.exec())
		return false;

	QString fileName = dialog.selectedFiles().at(0);
	DeckList::FileFormat fmt = DeckList::getFormatFromNameFilter(dialog.selectedNameFilter());

	if (deckModel->getDeckList()->saveToFile(fileName, fmt)) {
		lastFileName = fileName;
		lastFileFormat = fmt;
		setWindowModified(false);
		return true;
	}
	QMessageBox::critical(this, tr("Error"), tr("The deck could not be saved.\nPlease check that the directory is writable and try again."));
	return false;
}

void WndDeckEditor::actLoadDeckFromClipboard()
{
	if (!confirmClose())
		return;
	
	DlgLoadDeckFromClipboard dlg;
	if (!dlg.exec())
		return;
	
	setDeck(dlg.getDeckList());
	setWindowModified(true);
}

void WndDeckEditor::actSaveDeckToClipboard()
{
	QString buffer;
	QTextStream stream(&buffer);
	deckModel->getDeckList()->saveToStream_Plain(stream);
	QApplication::clipboard()->setText(buffer, QClipboard::Clipboard);
	QApplication::clipboard()->setText(buffer, QClipboard::Selection);
}

void WndDeckEditor::actPrintDeck()
{
	QPrintPreviewDialog *dlg = new QPrintPreviewDialog(this);
	connect(dlg, SIGNAL(paintRequested(QPrinter *)), deckModel, SLOT(printDeckList(QPrinter *)));
	dlg->exec();
}

void WndDeckEditor::actEditSets()
{
	WndSets *w = new WndSets;
	w->setWindowModality(Qt::WindowModal);
	w->show();
}

void WndDeckEditor::actSearch()
{
	if (dlgCardSearch->exec()) {
		searchEdit->clear();
		databaseDisplayModel->setCardName(dlgCardSearch->getCardName());
		databaseDisplayModel->setCardText(dlgCardSearch->getCardText());
		databaseDisplayModel->setCardTypes(dlgCardSearch->getCardTypes());
		databaseDisplayModel->setCardColors(dlgCardSearch->getCardColors());
	}
}

void WndDeckEditor::actClearSearch()
{
	databaseDisplayModel->clearSearch();
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
	const QString cardName = currentIndex.sibling(currentIndex.row(), 0).data().toString();

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

void WndDeckEditor::actUpdatePrices()
{
	aUpdatePrices->setDisabled(true);
	PriceUpdater *up = new PriceUpdater(deckModel->getDeckList());
	connect(up, SIGNAL(finishedUpdate()), this, SLOT(finishedUpdatingPrices()));
	up->updatePrices();
}

void WndDeckEditor::finishedUpdatingPrices()
{
	deckModel->pricesUpdated();
	setWindowModified(true);
	aUpdatePrices->setDisabled(false);
}

void WndDeckEditor::setDeck(DeckList *_deck, const QString &_lastFileName, DeckList::FileFormat _lastFileFormat)
{
	deckModel->setDeckList(_deck);

	lastFileName = _lastFileName;
	lastFileFormat = _lastFileFormat;
	nameEdit->setText(_deck->getName());
	commentsEdit->setText(_deck->getComments());
	updateHash();
	deckModel->sort(1);
	deckView->expandAll();
	setWindowModified(false);
	
	db->cacheCardPixmaps(_deck->getCardList());
	deckView->expandAll();
	setWindowModified(false);
}
