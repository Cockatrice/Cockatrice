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
#include <QProcessEnvironment>
#include <QTimer>
#include "tab_deck_editor.h"
#include "window_sets.h"
#include "carddatabase.h"
#include "carddatabasemodel.h"
#include "decklistmodel.h"
#include "cardinfowidget.h"
#include "dlg_load_deck_from_clipboard.h"
#include "dlg_edit_tokens.h"
#include "main.h"
#include "settingscache.h"
#include "priceupdater.h"
#include "tab_supervisor.h"
#include "deckstats_interface.h"
#include "abstractclient.h"
#include "pending_command.h"
#include "pb/response.pb.h"
#include "pb/command_deck_upload.pb.h"
#include "filtertreemodel.h"
#include "cardframe.h"
#include "filterbuilder.h"

void SearchLineEdit::keyPressEvent(QKeyEvent *event)
{
    if (treeView && ((event->key() == Qt::Key_Up) || (event->key() == Qt::Key_Down)))
        QCoreApplication::sendEvent(treeView, event);
    QLineEdit::keyPressEvent(event);
}

TabDeckEditor::TabDeckEditor(TabSupervisor *_tabSupervisor, QWidget *parent)
    : Tab(_tabSupervisor, parent), modified(false)
{
    aClearSearch = new QAction(QString(), this);
    aClearSearch->setIcon(QIcon("theme:icon_clearsearch.svg"));
    connect(aClearSearch, SIGNAL(triggered()), this, SLOT(actClearSearch()));
    searchEdit = new SearchLineEdit;
#if QT_VERSION >= 0x050300
    searchEdit->addAction(QIcon("theme:icon_search_black.svg"), QLineEdit::LeadingPosition);
#endif
    searchEdit->setObjectName("searchEdit");
    
    setFocusProxy(searchEdit);
    setFocusPolicy(Qt::ClickFocus);

    searchEdit->installEventFilter(&searchKeySignals);
    connect(searchEdit, SIGNAL(textChanged(const QString &)), this, SLOT(updateSearch(const QString &)));
    connect(&searchKeySignals, SIGNAL(onEnter()), this, SLOT(actAddCard()));
    connect(&searchKeySignals, SIGNAL(onCtrlAltEqual()), this, SLOT(actAddCard()));
    connect(&searchKeySignals, SIGNAL(onCtrlAltRBracket()), this, SLOT(actAddCardToSideboard()));
    connect(&searchKeySignals, SIGNAL(onCtrlAltMinus()), this, SLOT(actDecrementCard()));
    connect(&searchKeySignals, SIGNAL(onCtrlAltLBracket()), this, SLOT(actDecrementCardFromSideboard()));
    connect(&searchKeySignals, SIGNAL(onCtrlAltEnter()), this, SLOT(actAddCardToSideboard()));
    connect(&searchKeySignals, SIGNAL(onCtrlEnter()), this, SLOT(actAddCardToSideboard()));

    QToolBar *deckEditToolBar = new QToolBar;
    deckEditToolBar->setOrientation(Qt::Horizontal);
    deckEditToolBar->setIconSize(QSize(24, 24));

    QHBoxLayout *searchLayout = new QHBoxLayout;
    searchLayout->addWidget(deckEditToolBar);
    searchLayout->addWidget(searchEdit);

    databaseModel = new CardDatabaseModel(db, this);
    databaseDisplayModel = new CardDatabaseDisplayModel(this);
    databaseDisplayModel->setSourceModel(databaseModel);
    databaseDisplayModel->setFilterKeyColumn(0);
    databaseDisplayModel->sort(0, Qt::AscendingOrder);

    databaseView = new QTreeView();
    databaseView->setFocusProxy(searchEdit);
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

    cardInfo = new CardFrame(250, 356);
    aCardTextOnly = new QAction(QString(), this);
    aCardTextOnly->setCheckable(true);
    connect(aCardTextOnly, SIGNAL(triggered()), cardInfo, SLOT(toggleCardTextOnly()));

    filterModel = new FilterTreeModel();
    databaseDisplayModel->setFilterTree(filterModel->filterTree());
    filterView = new QTreeView;
    filterView->setModel(filterModel);
    filterView->setMaximumWidth(250);
    filterView->setUniformRowHeights(true);
    filterView->setHeaderHidden(true);
    filterView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(filterModel, SIGNAL(layoutChanged()), filterView, SLOT(expandAll()));
    connect(filterView, SIGNAL(customContextMenuRequested(const QPoint &)),
            this, SLOT(filterViewCustomContextMenu(const QPoint &)));
    FilterBuilder *filterBuilder = new FilterBuilder;
    filterBuilder->setMaximumWidth(250);
    connect(filterBuilder, SIGNAL(add(const CardFilter *)), filterModel, SLOT(addFilter(const CardFilter *)));

    QVBoxLayout *filter = new QVBoxLayout;
    filter->addWidget(filterBuilder, 0, Qt::AlignTop);
    filter->addWidget(filterView, 10);

    QVBoxLayout *middleFrame = new QVBoxLayout;
    middleFrame->addWidget(cardInfo, 0, Qt::AlignTop);
    middleFrame->addLayout(filter, 10);

    deckModel = new DeckListModel(this);
    connect(deckModel, SIGNAL(deckHashChanged()), this, SLOT(updateHash()));
    deckView = new QTreeView();
    deckView->setModel(deckModel);
    deckView->setUniformRowHeights(true);
    deckView->setSortingEnabled(true);
    deckView->sortByColumn(1, Qt::AscendingOrder);
#if QT_VERSION < 0x050000
    deckView->header()->setResizeMode(QHeaderView::ResizeToContents);
#else
    deckView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
#endif
    deckView->installEventFilter(&deckViewKeySignals);
    connect(deckView->selectionModel(), SIGNAL(currentRowChanged(const QModelIndex &, const QModelIndex &)), this, SLOT(updateCardInfoRight(const QModelIndex &, const QModelIndex &)));
    connect(deckView, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(actSwapCard()));
    connect(&deckViewKeySignals, SIGNAL(onS()), this, SLOT(actSwapCard()));
    connect(&deckViewKeySignals, SIGNAL(onEnter()), this, SLOT(actIncrement()));
    connect(&deckViewKeySignals, SIGNAL(onCtrlAltEqual()), this, SLOT(actIncrement()));
    connect(&deckViewKeySignals, SIGNAL(onCtrlAltMinus()), this, SLOT(actDecrement()));
    connect(&deckViewKeySignals, SIGNAL(onRight()), this, SLOT(actIncrement()));
    connect(&deckViewKeySignals, SIGNAL(onLeft()), this, SLOT(actDecrement()));
    connect(&deckViewKeySignals, SIGNAL(onDelete()), this, SLOT(actRemoveCard()));

    nameLabel = new QLabel();
    nameEdit = new QLineEdit;
    nameLabel->setBuddy(nameEdit);
    connect(nameEdit, SIGNAL(textChanged(const QString &)), this, SLOT(updateName(const QString &)));
    commentsLabel = new QLabel();
    commentsEdit = new QTextEdit;
    commentsEdit->setMaximumHeight(70);
    commentsLabel->setBuddy(commentsEdit);
    connect(commentsEdit, SIGNAL(textChanged()), this, SLOT(updateComments()));
    hashLabel1 = new QLabel();
    hashLabel = new QLabel;

    QGridLayout *grid = new QGridLayout;
    grid->addWidget(nameLabel, 0, 0);
    grid->addWidget(nameEdit, 0, 1);

    grid->addWidget(commentsLabel, 1, 0);
    grid->addWidget(commentsEdit, 1, 1);

    grid->addWidget(hashLabel1, 2, 0);
    grid->addWidget(hashLabel, 2, 1);

    /* Update price
    aUpdatePrices = new QAction(QString(), this);
    aUpdatePrices->setIcon(QIcon("theme:icon_update.png"));
    connect(aUpdatePrices, SIGNAL(triggered()), this, SLOT(actUpdatePrices()));
    if (!settingsCache->getPriceTagFeature())
        aUpdatePrices->setVisible(false);
    connect(settingsCache, SIGNAL(priceTagFeatureChanged(int)), this, SLOT(setPriceTagFeatureEnabled(int)));
    */

    QToolBar *deckToolBar = new QToolBar;
    deckToolBar->setOrientation(Qt::Vertical);
    deckToolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    deckToolBar->setIconSize(QSize(24, 24));
    //deckToolBar->addAction(aUpdatePrices);
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
    mainLayout->addLayout(rightFrame);
    setLayout(mainLayout);

    aNewDeck = new QAction(QString(), this);
    aNewDeck->setShortcuts(QKeySequence::New);
    connect(aNewDeck, SIGNAL(triggered()), this, SLOT(actNewDeck()));
    aLoadDeck = new QAction(QString(), this);
    aLoadDeck->setShortcuts(QKeySequence::Open);
    connect(aLoadDeck, SIGNAL(triggered()), this, SLOT(actLoadDeck()));
    aSaveDeck = new QAction(QString(), this);
    aSaveDeck->setShortcuts(QKeySequence::Save);
    connect(aSaveDeck, SIGNAL(triggered()), this, SLOT(actSaveDeck()));
    aSaveDeckAs = new QAction(QString(), this);
//    aSaveDeckAs->setShortcuts(QKeySequence::SaveAs);
    connect(aSaveDeckAs, SIGNAL(triggered()), this, SLOT(actSaveDeckAs()));
    aLoadDeckFromClipboard = new QAction(QString(), this);
    connect(aLoadDeckFromClipboard, SIGNAL(triggered()), this, SLOT(actLoadDeckFromClipboard()));
    aLoadDeckFromClipboard->setShortcuts(QKeySequence::Paste);
    aSaveDeckToClipboard = new QAction(QString(), this);
    connect(aSaveDeckToClipboard, SIGNAL(triggered()), this, SLOT(actSaveDeckToClipboard()));
    aSaveDeckToClipboard->setShortcuts(QKeySequence::Copy);
    aPrintDeck = new QAction(QString(), this);
    aPrintDeck->setShortcuts(QKeySequence::Print);
    connect(aPrintDeck, SIGNAL(triggered()), this, SLOT(actPrintDeck()));
    aAnalyzeDeck = new QAction(QString(), this);
    connect(aAnalyzeDeck, SIGNAL(triggered()), this, SLOT(actAnalyzeDeck()));
    aClose = new QAction(QString(), this);
    connect(aClose, SIGNAL(triggered()), this, SLOT(closeRequest()));
    aOpenCustomFolder = new QAction(QString(), this);
    connect(aOpenCustomFolder, SIGNAL(triggered()), this, SLOT(actOpenCustomFolder()));

    aEditSets = new QAction(QString(), this);
    connect(aEditSets, SIGNAL(triggered()), this, SLOT(actEditSets()));
    aEditTokens = new QAction(QString(), this);
    connect(aEditTokens, SIGNAL(triggered()), this, SLOT(actEditTokens()));

    deckMenu = new QMenu(this);
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
    deckMenu->addAction(aAnalyzeDeck);
    deckMenu->addSeparator();
    deckMenu->addAction(aClose);
    addTabMenu(deckMenu);

    dbMenu = new QMenu(this);
    dbMenu->addAction(aEditSets);
    dbMenu->addAction(aEditTokens);
    dbMenu->addSeparator();
    dbMenu->addAction(aClearSearch);
    dbMenu->addAction(aCardTextOnly);
#if defined(Q_OS_WIN) || defined(Q_OS_MAC)
    dbMenu->addSeparator();
    dbMenu->addAction(aOpenCustomFolder);
#endif
    addTabMenu(dbMenu);

    aAddCard = new QAction(QString(), this);
    aAddCard->setIcon(QIcon("theme:arrow_right_green.svg"));
    connect(aAddCard, SIGNAL(triggered()), this, SLOT(actAddCard()));
    aAddCardToSideboard = new QAction(QString(), this);
    aAddCardToSideboard->setIcon(QIcon("theme:add_to_sideboard.svg"));
    connect(aAddCardToSideboard, SIGNAL(triggered()), this, SLOT(actAddCardToSideboard()));
    aRemoveCard = new QAction(QString(), this);
    aRemoveCard->setIcon(QIcon("theme:remove_row.svg"));
    connect(aRemoveCard, SIGNAL(triggered()), this, SLOT(actRemoveCard()));
    aIncrement = new QAction(QString(), this);
    aIncrement->setIcon(QIcon("theme:increment.svg"));
    connect(aIncrement, SIGNAL(triggered()), this, SLOT(actIncrement()));
    aDecrement = new QAction(QString(), this);
    aDecrement->setIcon(QIcon("theme:decrement.svg"));
    connect(aDecrement, SIGNAL(triggered()), this, SLOT(actDecrement()));

    deckEditToolBar->addAction(aAddCard);
    deckEditToolBar->addAction(aAddCardToSideboard);
    deckEditToolBar->addAction(aRemoveCard);
    deckEditToolBar->addAction(aDecrement);
    deckEditToolBar->addAction(aIncrement);
    deckEditToolBar->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    retranslateUi();
    
    resize(950, 700);

    QTimer::singleShot(0, this, SLOT(checkFirstRunDetected()));
}

TabDeckEditor::~TabDeckEditor()
{
    emit deckEditorClosing(this);
}

void TabDeckEditor::retranslateUi()
{
    aCardTextOnly->setText(tr("Show card text only"));
    aClearSearch->setText(tr("&Clear search"));
    
    nameLabel->setText(tr("Deck &name:"));
    commentsLabel->setText(tr("&Comments:"));
    hashLabel1->setText(tr("Hash:"));
    
    //aUpdatePrices->setText(tr("&Update prices"));
    //aUpdatePrices->setShortcut(QKeySequence("Ctrl+U"));

    aNewDeck->setText(tr("&New deck"));
    aLoadDeck->setText(tr("&Load deck..."));
    aSaveDeck->setText(tr("&Save deck"));
    aSaveDeckAs->setText(tr("Save deck &as..."));
    aLoadDeckFromClipboard->setText(tr("Load deck from cl&ipboard..."));
    aSaveDeckToClipboard->setText(tr("Save deck to clip&board"));
    aPrintDeck->setText(tr("&Print deck..."));
    aAnalyzeDeck->setText(tr("&Analyze deck on deckstats.net"));
    aOpenCustomFolder->setText(tr("Open custom image folder"));
    aClose->setText(tr("&Close"));
    aClose->setShortcut(QKeySequence("Ctrl+Q"));
    
    aAddCard->setText(tr("Add card to &maindeck"));
    aAddCardToSideboard->setText(tr("Add card to &sideboard"));

    aRemoveCard->setText(tr("&Remove row"));
    aRemoveCard->setShortcut(QKeySequence("Del"));
    aIncrement->setText(tr("&Increment number"));
    aIncrement->setShortcut(QKeySequence("+"));
    aDecrement->setText(tr("&Decrement number"));
    aDecrement->setShortcut(QKeySequence("-"));
    
    deckMenu->setTitle(tr("&Deck Editor"));
    dbMenu->setTitle(tr("C&ard Database"));
    
    aEditSets->setText(tr("&Edit sets..."));
    aEditTokens->setText(tr("Edit &tokens..."));
}

QString TabDeckEditor::getTabText() const
{
    QString result = tr("Deck: %1").arg(nameEdit->text());
    if (modified)
        result.prepend("* ");
    return result;
}

void TabDeckEditor::updateName(const QString &name)
{
    deckModel->getDeckList()->setName(name);
    setModified(true);
}

void TabDeckEditor::updateComments()
{
    deckModel->getDeckList()->setComments(commentsEdit->toPlainText());
    setModified(true);
}

void TabDeckEditor::updateCardInfoLeft(const QModelIndex &current, const QModelIndex &/*previous*/)
{
    cardInfo->setCard(current.sibling(current.row(), 0).data().toString());
}

void TabDeckEditor::updateCardInfoRight(const QModelIndex &current, const QModelIndex &/*previous*/)
{
    if (!current.isValid())
        return;
    if (!current.model()->hasChildren(current.sibling(current.row(), 0)))
        cardInfo->setCard(current.sibling(current.row(), 1).data().toString());
}

void TabDeckEditor::updateSearch(const QString &search)
{
    databaseDisplayModel->setCardName(search);
    QModelIndexList sel = databaseView->selectionModel()->selectedRows();
    if (sel.isEmpty() && databaseDisplayModel->rowCount())
        databaseView->selectionModel()->setCurrentIndex(databaseDisplayModel->index(0, 0), QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
}

void TabDeckEditor::updateHash()
{
    hashLabel->setText(deckModel->getDeckList()->getDeckHash());
}

bool TabDeckEditor::confirmClose()
{
    if (modified) {
        tabSupervisor->setCurrentWidget(this);
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

void TabDeckEditor::closeRequest()
{
    if (confirmClose())
        deleteLater();
}

void TabDeckEditor::actNewDeck()
{
    if (!confirmClose())
        return;

    deckModel->cleanList();
    nameEdit->setText(QString());
    commentsEdit->setText(QString());
    hashLabel->setText(QString());
    setModified(false);
}

void TabDeckEditor::actLoadDeck()
{
    if (!confirmClose())
        return;

    QFileDialog dialog(this, tr("Load deck"));
    dialog.setDirectory(settingsCache->getDeckPath());
    dialog.setNameFilters(DeckLoader::fileNameFilters);
    if (!dialog.exec())
        return;

    QString fileName = dialog.selectedFiles().at(0);
    DeckLoader::FileFormat fmt = DeckLoader::getFormatFromName(fileName);
    
    DeckLoader *l = new DeckLoader;
    if (l->loadFromFile(fileName, fmt))
        setDeck(l);
    else
        delete l;
}

void TabDeckEditor::saveDeckRemoteFinished(const Response &response)
{
    if (response.response_code() != Response::RespOk)
        QMessageBox::critical(this, tr("Error"), tr("The deck could not be saved."));
    else
        setModified(false);
}

bool TabDeckEditor::actSaveDeck()
{
    DeckLoader *const deck = deckModel->getDeckList();
    if (deck->getLastRemoteDeckId() != -1) {
        Command_DeckUpload cmd;
        cmd.set_deck_id(deck->getLastRemoteDeckId());
        cmd.set_deck_list(deck->writeToString_Native().toStdString());
        
        PendingCommand *pend = AbstractClient::prepareSessionCommand(cmd);
        connect(pend, SIGNAL(finished(Response, CommandContainer, QVariant)), this, SLOT(saveDeckRemoteFinished(Response)));
        tabSupervisor->getClient()->sendCommand(pend);
        
        return true;
    } else if (deck->getLastFileName().isEmpty())
        return actSaveDeckAs();
    else if (deck->saveToFile(deck->getLastFileName(), deck->getLastFileFormat())) {
        setModified(false);
        return true;
    }
    QMessageBox::critical(this, tr("Error"), tr("The deck could not be saved.\nPlease check that the directory is writable and try again."));
    return false;
}

bool TabDeckEditor::actSaveDeckAs()
{
    QFileDialog dialog(this, tr("Save deck"));
    dialog.setDirectory(settingsCache->getDeckPath());
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setConfirmOverwrite(true);
    dialog.setDefaultSuffix("cod");
    dialog.setNameFilters(DeckLoader::fileNameFilters);
    dialog.selectFile(deckModel->getDeckList()->getName());
    if (!dialog.exec())
        return false;

    QString fileName = dialog.selectedFiles().at(0);
    DeckLoader::FileFormat fmt = DeckLoader::getFormatFromName(fileName);

    if (!deckModel->getDeckList()->saveToFile(fileName, fmt)) {
        QMessageBox::critical(this, tr("Error"), tr("The deck could not be saved.\nPlease check that the directory is writable and try again."));
        return false;
    }
    setModified(false);
    return true;
}

void TabDeckEditor::actLoadDeckFromClipboard()
{
    if (!confirmClose())
        return;
    
    DlgLoadDeckFromClipboard dlg;
    if (!dlg.exec())
        return;
    
    setDeck(dlg.getDeckList());
    setModified(true);
}

void TabDeckEditor::actSaveDeckToClipboard()
{
    QString buffer;
    QTextStream stream(&buffer);
    deckModel->getDeckList()->saveToStream_Plain(stream);
    QApplication::clipboard()->setText(buffer, QClipboard::Clipboard);
    QApplication::clipboard()->setText(buffer, QClipboard::Selection);
}

void TabDeckEditor::actPrintDeck()
{
    QPrintPreviewDialog *dlg = new QPrintPreviewDialog(this);
    connect(dlg, SIGNAL(paintRequested(QPrinter *)), deckModel, SLOT(printDeckList(QPrinter *)));
    dlg->exec();
}

void TabDeckEditor::actAnalyzeDeck()
{
    DeckStatsInterface *interface = new DeckStatsInterface(
        *databaseModel->getDatabase(),
        this
    ); // it deletes itself when done
    interface->analyzeDeck(deckModel->getDeckList());
}


void TabDeckEditor::actOpenCustomFolder() {

#if defined(Q_OS_MAC)

    QStringList scriptArgs;
    scriptArgs << QLatin1String("-e");
    scriptArgs << QString::fromLatin1("tell application \"Finder\" to open POSIX file \"%1\"").arg(settingsCache->getPicsPath() + "/custom/");
    scriptArgs << QLatin1String("-e");
    scriptArgs << QLatin1String("tell application \"Finder\" to activate");

    QProcess::execute("/usr/bin/osascript", scriptArgs);
#endif
#if defined(Q_OS_WIN)
    QStringList args;
    QString pathToFolder = settingsCache->getPicsPath().append("/custom");
    args << QDir::toNativeSeparators(pathToFolder);
    QProcess::startDetached("explorer", args);
#endif

}

void TabDeckEditor::actEditSets()
{
    WndSets *w = new WndSets;
    w->setWindowModality(Qt::WindowModal);
    w->show();
}

void TabDeckEditor::actEditTokens()
{
    DlgEditTokens dlg(databaseModel);
    dlg.exec();
    db->saveToFile(settingsCache->getTokenDatabasePath(), true);
}

void TabDeckEditor::actClearSearch()
{
    databaseDisplayModel->clearSearch();
}

void TabDeckEditor::recursiveExpand(const QModelIndex &index)
{
    if (index.parent().isValid())
        recursiveExpand(index.parent());
    deckView->expand(index);
}

CardInfo *TabDeckEditor::currentCardInfo() const
{
    const QModelIndex currentIndex = databaseView->selectionModel()->currentIndex();
    if (!currentIndex.isValid())
        return NULL;
    const QString cardName = currentIndex.sibling(currentIndex.row(), 0).data().toString();
    
    return db->getCard(cardName);
}

void TabDeckEditor::addCardHelper(QString zoneName)
{
    const CardInfo *info;

    info = currentCardInfo();
    if(!info)
        return;
    if (info->getIsToken())
        zoneName = "tokens";

    QModelIndex newCardIndex = deckModel->addCard(info->getName(), zoneName);
    recursiveExpand(newCardIndex);
    deckView->setCurrentIndex(newCardIndex);
    setModified(true);
}

void TabDeckEditor::actSwapCard()
{
    const QModelIndex currentIndex = deckView->selectionModel()->currentIndex();
    if (!currentIndex.isValid())
        return;
    const QString cardName = currentIndex.sibling(currentIndex.row(), 1).data().toString();
    const QModelIndex gparent = currentIndex.parent().parent();
    if (!gparent.isValid())
            return;

    const QString zoneName = gparent.sibling(gparent.row(), 1).data().toString();
    actDecrement();

    const QString otherZoneName = zoneName == "Maindeck" ? "side" : "main";

    QModelIndex newCardIndex = deckModel->addCard(cardName, otherZoneName);
    recursiveExpand(newCardIndex);
    deckView->setCurrentIndex(newCardIndex);
    setModified(true);
}

void TabDeckEditor::actAddCard()
{
    addCardHelper("main");
}

void TabDeckEditor::actAddCardToSideboard()
{
    addCardHelper("side");
}

void TabDeckEditor::actRemoveCard()
{
    const QModelIndex &currentIndex = deckView->selectionModel()->currentIndex();
    if (!currentIndex.isValid() || deckModel->hasChildren(currentIndex))
        return;
    deckModel->removeRow(currentIndex.row(), currentIndex.parent());
    setModified(true);
}

void TabDeckEditor::offsetCountAtIndex(const QModelIndex &idx, int offset)
{
    if (!idx.isValid() || offset == 0)
        return;

    const QModelIndex numberIndex = idx.sibling(idx.row(), 0);
    const int count = deckModel->data(numberIndex, Qt::EditRole).toInt();
    const int new_count = count + offset;
    deckView->setCurrentIndex(numberIndex);
    if (new_count <= 0)
        deckModel->removeRow(idx.row(), idx.parent());
    else
        deckModel->setData(numberIndex, new_count, Qt::EditRole);
    setModified(true);
}

void TabDeckEditor::decrementCardHelper(QString zoneName)
{
    const CardInfo *info;
    QModelIndex idx;

    info = currentCardInfo();
    if(!info)
        return;
    if (info->getIsToken())
        zoneName = "tokens";

    idx = deckModel->findCard(info->getName(), zoneName);
    offsetCountAtIndex(idx, -1);
}

void TabDeckEditor::actDecrementCard()
{
    decrementCardHelper("main");
}

void TabDeckEditor::actDecrementCardFromSideboard()
{
    decrementCardHelper("side");
}

void TabDeckEditor::actIncrement()
{
    const QModelIndex &currentIndex = deckView->selectionModel()->currentIndex();
    offsetCountAtIndex(currentIndex, 1);
}

void TabDeckEditor::actDecrement()
{
    const QModelIndex &currentIndex = deckView->selectionModel()->currentIndex();
    offsetCountAtIndex(currentIndex, -1);
}

void TabDeckEditor::setPriceTagFeatureEnabled(int /* enabled */)
{
    //aUpdatePrices->setVisible(enabled);
    deckModel->pricesUpdated();
}


/*
void TabDeckEditor::actUpdatePrices()
{
    aUpdatePrices->setDisabled(true);
    AbstractPriceUpdater *up;

    switch(settingsCache->getPriceTagSource())
    {
        case AbstractPriceUpdater::DBPriceSource:
        default:
            up = new DBPriceUpdater(deckModel->getDeckList());
            break;
    }
     
    connect(up, SIGNAL(finishedUpdate()), this, SLOT(finishedUpdatingPrices()));
    up->updatePrices();
}
*/


void TabDeckEditor::finishedUpdatingPrices()
{
    //deckModel->pricesUpdated();
    //setModified(true);
    //aUpdatePrices->setDisabled(false);
}

void TabDeckEditor::setDeck(DeckLoader *_deck)
{
    deckModel->setDeckList(_deck);

    nameEdit->setText(deckModel->getDeckList()->getName());
    commentsEdit->setText(deckModel->getDeckList()->getComments());
    updateHash();
    deckModel->sort(deckView->header()->sortIndicatorSection(), deckView->header()->sortIndicatorOrder());
    deckView->expandAll();
    setModified(false);

    db->cacheCardPixmaps(deckModel->getDeckList()->getCardList());
    deckView->expandAll();
    setModified(false);
}

void TabDeckEditor::setModified(bool _modified)
{
    modified = _modified;
    emit tabTextChanged(this, getTabText());
}

void TabDeckEditor::filterViewCustomContextMenu(const QPoint &point) {
    QMenu menu;
    QAction *action;
    QModelIndex idx;

    idx = filterView->indexAt(point);
    if (!idx.isValid())
        return;

    action = menu.addAction(QString("delete"));
    action->setData(point);
    connect(&menu, SIGNAL(triggered(QAction *)),
            this, SLOT(filterRemove(QAction *)));
    menu.exec(filterView->mapToGlobal(point));
}

void TabDeckEditor::filterRemove(QAction *action) {
    QPoint point;
    QModelIndex idx;

    point = action->data().toPoint();
    idx = filterView->indexAt(point);
    if (!idx.isValid())
        return;

    filterModel->removeRow(idx.row(), idx.parent());
}

void TabDeckEditor::checkFirstRunDetected()
{
    if(db->hasDetectedFirstRun())
    {
        QMessageBox::information(this, tr("Welcome"), tr("Hi! Its seems like it's the first time you run this version of Cockatrice.\nAll the sets in the card database have been enabled.\nRead more about changing the set order or disabling specific sets in the the \"Edit Sets\" window."));
        actEditSets();
    }
}