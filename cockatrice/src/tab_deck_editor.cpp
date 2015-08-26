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
#include <QGroupBox>
#include <QMenuBar>
#include <QMessageBox>
#include <QPrintPreviewDialog>
#include <QHeaderView>
#include <QApplication>
#include <QClipboard>
#include <QTextStream>
#include <QProcessEnvironment>
#include <QTimer>
#include <QDockWidget>
#include <QPushButton>
#include <QDir>
#include <QDesktopServices>
#include "tab_deck_editor.h"
#include "window_sets.h"
#include "carddatabase.h"
#include "pictureloader.h"
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

void TabDeckEditor::createShowHideDocksButtons()
{
    btnFilter = new QPushButton(QPixmap("theme:icons/view"),QString());
    btnFilter->setObjectName("btnFilter");
    btnFilter->setCheckable(true);
    btnFilter->setChecked(true);
    btnFilter->setMaximumWidth(30);
    searchLayout->addWidget(btnFilter);

    btnDeck = new QPushButton(QPixmap("theme:hand"),QString());
    btnDeck->setObjectName("btnDeck");
    btnDeck->setCheckable(true);
    btnDeck->setChecked(true);
    btnDeck->setMaximumWidth(30);
    searchLayout->addWidget(btnDeck);

    btnCard = new QPushButton(QPixmap("theme:cardback"),QString());
    btnCard->setObjectName("btnCard");
    btnCard->setCheckable(true);
    btnCard->setChecked(true);
    btnCard->setMaximumWidth(30);
    searchLayout->addWidget(btnCard);
}

void TabDeckEditor::createDeckDock()
{
    deckModel = new DeckListModel(this);
    deckModel->setObjectName("deckModel");
    connect(deckModel, SIGNAL(deckHashChanged()), this, SLOT(updateHash()));
    deckView = new QTreeView();
    deckView->setObjectName("deckView");
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
    nameLabel->setObjectName("nameLabel");
    nameEdit = new QLineEdit;
    nameEdit->setObjectName("nameEdit");
    nameLabel->setBuddy(nameEdit);
    connect(nameEdit, SIGNAL(textChanged(const QString &)), this, SLOT(updateName(const QString &)));
    commentsLabel = new QLabel();
    commentsLabel->setObjectName("commentsLabel");
    commentsEdit = new QTextEdit;
    commentsEdit->setObjectName("commentsEdit");
    commentsEdit->setMaximumHeight(70);
    commentsLabel->setBuddy(commentsEdit);
    connect(commentsEdit, SIGNAL(textChanged()), this, SLOT(updateComments()));

    hashLabel1 = new QLabel();
    hashLabel1->setObjectName("hashLabel1");
    hashLabel = new QLabel;
    hashLabel->setObjectName("hashLabel");

    QGridLayout *grid = new QGridLayout;
    grid->setObjectName("grid");
    grid->addWidget(nameLabel, 0, 0);
    grid->addWidget(nameEdit, 0, 1);

    grid->addWidget(commentsLabel, 1, 0);
    grid->addWidget(commentsEdit, 1, 1);

    grid->addWidget(hashLabel1, 2, 0);
    grid->addWidget(hashLabel, 2, 1);

    /* Update price
    aUpdatePrices = new QAction(QString(), this);
    aUpdatePrices->setIcon(QPixmap("theme:icons/update"));
    connect(aUpdatePrices, SIGNAL(triggered()), this, SLOT(actUpdatePrices()));
    if (!settingsCache->getPriceTagFeature())
        aUpdatePrices->setVisible(false);
    connect(settingsCache, SIGNAL(priceTagFeatureChanged(int)), this, SLOT(setPriceTagFeatureEnabled(int)));
    */

    QToolBar *deckToolBar = new QToolBar;
    deckToolBar->setObjectName("deckToolBar");
    deckToolBar->setOrientation(Qt::Vertical);
    deckToolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    deckToolBar->setIconSize(QSize(24, 24));
    //deckToolBar->addAction(aUpdatePrices);
    QHBoxLayout *deckToolbarLayout = new QHBoxLayout;
    deckToolbarLayout->setObjectName("deckToolbarLayout");
    deckToolbarLayout->addStretch();
    deckToolbarLayout->addWidget(deckToolBar);
    deckToolbarLayout->addStretch();

    QVBoxLayout *rightFrame = new QVBoxLayout;
    rightFrame->setObjectName("rightFrame");
    rightFrame->addLayout(grid);
    rightFrame->addWidget(deckView, 10);
    rightFrame->addLayout(deckToolbarLayout);

    deckDock = new QDockWidget(MainWindow);
    deckDock->setObjectName("deckDock");

    deckDock->setMinimumSize(QSize(200, 41));
    deckDock->setAllowedAreas(Qt::LeftDockWidgetArea|Qt::RightDockWidgetArea);
    deckDock->setFeatures(QDockWidget::DockWidgetClosable|QDockWidget::DockWidgetFloatable|QDockWidget::DockWidgetMovable);
    QWidget *deckDockContents = new QWidget();
    deckDockContents->setObjectName("deckDockContents");
    deckDockContents->setLayout(rightFrame);
    deckDock->setWidget(deckDockContents);

    connect(btnDeck,SIGNAL(toggled(bool)),deckDock,SLOT(setVisible(bool)));
    deckDock->installEventFilter(this);
}

void TabDeckEditor::createCardInfoDock()
{
    cardInfo = new CardFrame();
    cardInfo->setObjectName("cardInfo");
    QVBoxLayout *cardInfoFrame = new QVBoxLayout;
    cardInfoFrame->setObjectName("cardInfoFrame");
    cardInfoFrame->addWidget(cardInfo);

    cardInfoDock = new QDockWidget(MainWindow);
    cardInfoDock->setObjectName("cardInfoDock");    

    cardInfoDock->setMinimumSize(QSize(200, 41));
    cardInfoDock->setAllowedAreas(Qt::LeftDockWidgetArea|Qt::RightDockWidgetArea);
    cardInfoDock->setFeatures(QDockWidget::DockWidgetClosable|QDockWidget::DockWidgetFloatable|QDockWidget::DockWidgetMovable);
    QWidget *cardInfoDockContents = new QWidget();
    cardInfoDockContents->setObjectName("cardInfoDockContents");
    cardInfoDockContents->setLayout(cardInfoFrame);
    cardInfoDock->setWidget(cardInfoDockContents);

    connect(btnCard,SIGNAL(toggled(bool)),cardInfoDock,SLOT(setVisible(bool)));
    cardInfoDock->installEventFilter(this);    
}

void TabDeckEditor::createFiltersDock()
{
    filterModel = new FilterTreeModel();
    filterModel->setObjectName("filterModel");
    databaseDisplayModel->setFilterTree(filterModel->filterTree());
    databaseDisplayModel->setObjectName("databaseDisplayModel");
    filterView = new QTreeView;
    filterView->setObjectName("filterView");
    filterView->setModel(filterModel);
    filterView->setUniformRowHeights(true);
    filterView->setHeaderHidden(true);
    filterView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(filterModel, SIGNAL(layoutChanged()), filterView, SLOT(expandAll()));
    connect(filterView, SIGNAL(customContextMenuRequested(const QPoint &)),this, SLOT(filterViewCustomContextMenu(const QPoint &)));

    FilterBuilder *filterBuilder = new FilterBuilder;
    filterBuilder->setObjectName("filterBuilder");
    connect(filterBuilder, SIGNAL(add(const CardFilter *)), filterModel, SLOT(addFilter(const CardFilter *)));

    QToolButton *filterDelOne = new QToolButton();
    filterDelOne->setObjectName("filterDelOne");
    filterDelOne->setDefaultAction(aClearFilterOne);
    filterDelOne->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    QToolButton *filterDelAll = new QToolButton();
    filterDelAll->setObjectName("filterDelAll");
    filterDelAll->setDefaultAction(aClearFilterAll);
    filterDelAll->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    QGridLayout *filterLayout = new QGridLayout;
    filterLayout->setObjectName("filterLayout");
    filterLayout->setContentsMargins(0,0,0,0);
    filterLayout->addWidget(filterBuilder, 0, 0, 1, 3);
    filterLayout->addWidget(filterView, 1, 0, 1, 3);
    filterLayout->addWidget(filterDelOne, 2, 0, 1, 1);
    filterLayout->addWidget(filterDelAll, 2, 2, 1, 1);

    filterBox = new QWidget();
    filterBox->setObjectName("filterBox");
    filterBox->setLayout(filterLayout);

    QVBoxLayout *filterFrame = new QVBoxLayout;
    filterFrame->setObjectName("filterFrame");
    filterFrame->addWidget(filterBox);

    filterDock = new QDockWidget(MainWindow);
    filterDock->setObjectName("filterDock");

    filterDock->setFeatures(QDockWidget::DockWidgetClosable|QDockWidget::DockWidgetFloatable|QDockWidget::DockWidgetMovable);
    QWidget *filterDockContents = new QWidget(MainWindow);
    filterDockContents->setObjectName("filterDockContents");
    filterDockContents->setLayout(filterFrame);
    filterDock->setWidget(filterDockContents);    

    connect(btnFilter,SIGNAL(toggled(bool)),filterDock,SLOT(setVisible(bool)));
    filterDock->installEventFilter(this);
}

void TabDeckEditor::createMenus()
{
    aNewDeck = new QAction(QString(), this);
    connect(aNewDeck, SIGNAL(triggered()), this, SLOT(actNewDeck()));

    aLoadDeck = new QAction(QString(), this);
    connect(aLoadDeck, SIGNAL(triggered()), this, SLOT(actLoadDeck()));

    aSaveDeck = new QAction(QString(), this);
    connect(aSaveDeck, SIGNAL(triggered()), this, SLOT(actSaveDeck()));

    aSaveDeckAs = new QAction(QString(), this);
    connect(aSaveDeckAs, SIGNAL(triggered()), this, SLOT(actSaveDeckAs()));

    aOpenCustomsetsFolder = new QAction(QString(), this);
        connect(aOpenCustomsetsFolder, SIGNAL(triggered()), this, SLOT(actOpenCustomsetsFolder()));

    aLoadDeckFromClipboard = new QAction(QString(), this);
    connect(aLoadDeckFromClipboard, SIGNAL(triggered()), this, SLOT(actLoadDeckFromClipboard()));

    aSaveDeckToClipboard = new QAction(QString(), this);
    connect(aSaveDeckToClipboard, SIGNAL(triggered()), this, SLOT(actSaveDeckToClipboard()));

    aPrintDeck = new QAction(QString(), this);
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

    aResetLayout = new QAction(QString(), this);
    connect(aResetLayout,SIGNAL(triggered()),this,SLOT(restartLayout()));

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
    deckMenu->addAction(aResetLayout);
    deckMenu->addSeparator();
    deckMenu->addAction(aClose);
    addTabMenu(deckMenu);

    aClearFilterAll = new QAction(QString(), this);
    aClearFilterAll->setIcon(QPixmap("theme:icons/clearsearch"));
    connect(aClearFilterAll, SIGNAL(triggered()), this, SLOT(actClearFilterAll()));

    aClearFilterOne = new QAction(QString(), this);
    aClearFilterOne->setIcon(QPixmap("theme:icons/decrement"));
    connect(aClearFilterOne, SIGNAL(triggered()), this, SLOT(actClearFilterOne()));

    dbMenu = new QMenu(this);
    dbMenu->addAction(aEditSets);
    dbMenu->addAction(aEditTokens);
    dbMenu->addSeparator();
    dbMenu->addAction(aClearFilterOne);
    dbMenu->addAction(aClearFilterAll);
#if defined(Q_OS_WIN) || defined(Q_OS_MAC)
    dbMenu->addSeparator();
    dbMenu->addAction(aOpenCustomFolder);
    dbMenu->addAction(aOpenCustomsetsFolder);
#endif
    addTabMenu(dbMenu);
}

void TabDeckEditor::createCentralFrame()
{
    searchEdit = new SearchLineEdit;
    searchEdit->setObjectName("searchEdit");
#if QT_VERSION >= 0x050300
    searchEdit->addAction(QPixmap("theme:icons/search"), QLineEdit::LeadingPosition);
#endif

    setFocusProxy(searchEdit);
    setFocusPolicy(Qt::ClickFocus);

    searchEdit->installEventFilter(&searchKeySignals);
    searchKeySignals.setObjectName("searchKeySignals");
    connect(searchEdit, SIGNAL(textChanged(const QString &)), this, SLOT(updateSearch(const QString &)));
    connect(&searchKeySignals, SIGNAL(onEnter()), this, SLOT(actAddCard()));
    connect(&searchKeySignals, SIGNAL(onCtrlAltEqual()), this, SLOT(actAddCard()));
    connect(&searchKeySignals, SIGNAL(onCtrlAltRBracket()), this, SLOT(actAddCardToSideboard()));
    connect(&searchKeySignals, SIGNAL(onCtrlAltMinus()), this, SLOT(actDecrementCard()));
    connect(&searchKeySignals, SIGNAL(onCtrlAltLBracket()), this, SLOT(actDecrementCardFromSideboard()));
    connect(&searchKeySignals, SIGNAL(onCtrlAltEnter()), this, SLOT(actAddCardToSideboard()));
    connect(&searchKeySignals, SIGNAL(onCtrlEnter()), this, SLOT(actAddCardToSideboard()));    

    databaseModel = new CardDatabaseModel(db, this);
    databaseModel->setObjectName("databaseModel");
    databaseDisplayModel = new CardDatabaseDisplayModel(this);
    databaseDisplayModel->setSourceModel(databaseModel);
    databaseDisplayModel->setFilterKeyColumn(0);

    databaseView = new QTreeView();
    databaseView->setObjectName("databaseView");
    databaseView->setFocusProxy(searchEdit);
    databaseView->setUniformRowHeights(true);
    databaseView->setRootIsDecorated(false);
    databaseView->setAlternatingRowColors(true);
    databaseView->setSortingEnabled(true);
    databaseView->sortByColumn(0, Qt::AscendingOrder);
    databaseView->setModel(databaseDisplayModel);
    databaseView->resizeColumnToContents(0);
    connect(databaseView->selectionModel(), SIGNAL(currentRowChanged(const QModelIndex &, const QModelIndex &)), this, SLOT(updateCardInfoLeft(const QModelIndex &, const QModelIndex &)));
    connect(databaseView, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(actAddCard()));
    searchEdit->setTreeView(databaseView);

    aAddCard = new QAction(QString(), this);
    aAddCard->setIcon(QPixmap("theme:icons/arrow_right_green"));
    connect(aAddCard, SIGNAL(triggered()), this, SLOT(actAddCard()));
    aAddCardToSideboard = new QAction(QString(), this);
    aAddCardToSideboard->setIcon(QPixmap("theme:icons/arrow_right_blue"));
    connect(aAddCardToSideboard, SIGNAL(triggered()), this, SLOT(actAddCardToSideboard()));
    aRemoveCard = new QAction(QString(), this);
    aRemoveCard->setIcon(QPixmap("theme:icons/remove_row"));
    connect(aRemoveCard, SIGNAL(triggered()), this, SLOT(actRemoveCard()));
    aIncrement = new QAction(QString(), this);
    aIncrement->setIcon(QPixmap("theme:icons/increment"));
    connect(aIncrement, SIGNAL(triggered()), this, SLOT(actIncrement()));
    aDecrement = new QAction(QString(), this);
    aDecrement->setIcon(QPixmap("theme:icons/decrement"));
    connect(aDecrement, SIGNAL(triggered()), this, SLOT(actDecrement()));

    QToolBar *deckEditToolBar = new QToolBar;
    deckEditToolBar->setObjectName("deckEditToolBar");
    deckEditToolBar->setOrientation(Qt::Horizontal);
    deckEditToolBar->setIconSize(QSize(24, 24));

    deckEditToolBar->addAction(aAddCard);
    deckEditToolBar->addAction(aAddCardToSideboard);
    deckEditToolBar->addAction(aRemoveCard);
    deckEditToolBar->addAction(aDecrement);
    deckEditToolBar->addAction(aIncrement);
    deckEditToolBar->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    searchLayout = new QHBoxLayout;
    searchLayout->setObjectName("searchLayout");
    searchLayout->addWidget(deckEditToolBar);
    searchLayout->addWidget(searchEdit);
    createShowHideDocksButtons();

    centralFrame = new QVBoxLayout;
    centralFrame->setObjectName("centralFrame");
    centralFrame->addLayout(searchLayout);
    centralFrame->addWidget(databaseView);

    centralWidget = new QWidget(MainWindow);
    centralWidget->setObjectName("centralWidget");
    centralWidget->setLayout(centralFrame);
    MainWindow->setCentralWidget(centralWidget);
    MainWindow->setDockOptions(QMainWindow::AnimatedDocks|QMainWindow::AllowNestedDocks|QMainWindow::AllowTabbedDocks);

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->setObjectName("mainLayout");
    mainLayout->addWidget(MainWindow);
    setLayout(mainLayout);
}

void TabDeckEditor::restartLayout()
{
    btnDeck->setChecked(true);
    btnFilter->setChecked(true);
    btnCard->setChecked(true);

    deckDock->setFloating(false);
    cardInfoDock->setFloating(false);
    filterDock->setFloating(false);

    MainWindow->addDockWidget(static_cast<Qt::DockWidgetArea>(2), deckDock);
    MainWindow->addDockWidget(static_cast<Qt::DockWidgetArea>(2), cardInfoDock);
    MainWindow->addDockWidget(static_cast<Qt::DockWidgetArea>(2), filterDock);

    MainWindow->splitDockWidget(cardInfoDock, deckDock, Qt::Horizontal);
    MainWindow->splitDockWidget(cardInfoDock, filterDock, Qt::Vertical);

    deckDock->setMinimumWidth(360);
    deckDock->setMaximumWidth(360);

    cardInfoDock->setMinimumSize(250, 360);
    cardInfoDock->setMaximumSize(250, 360);
    QTimer::singleShot(100, this, SLOT(freeDocksSize()));
}

void TabDeckEditor::freeDocksSize()
{
    deckDock->setMinimumSize(100, 100);
    deckDock->setMaximumSize(5000, 5000);

    cardInfoDock->setMinimumSize(100, 100);
    cardInfoDock->setMaximumSize(5000, 5000);

    filterDock->setMinimumSize(100,100);
    filterDock->setMaximumSize(5000,5000);
}

void TabDeckEditor::refreshShortcuts()
{
    aNewDeck->setShortcuts(settingsCache->shortcuts().getShortcut("TabDeckEditor/aNewDeck"));
    aLoadDeck->setShortcuts(settingsCache->shortcuts().getShortcut("TabDeckEditor/aLoadDeck"));
    aSaveDeck->setShortcuts(settingsCache->shortcuts().getShortcut("TabDeckEditor/aSaveDeck"));
    aSaveDeckAs->setShortcuts(settingsCache->shortcuts().getShortcut("TabDeckEditor/aSaveDeckAs"));
    aLoadDeckFromClipboard->setShortcuts(settingsCache->shortcuts().getShortcut("TabDeckEditor/aLoadDeckFromClipboard"));
    aPrintDeck->setShortcuts(settingsCache->shortcuts().getShortcut("TabDeckEditor/aPrintDeck"));
    aAnalyzeDeck->setShortcuts(settingsCache->shortcuts().getShortcut("TabDeckEditor/aAnalyzeDeck"));
    aClose->setShortcuts(settingsCache->shortcuts().getShortcut("TabDeckEditor/aClose"));
    aOpenCustomFolder->setShortcuts(settingsCache->shortcuts().getShortcut("TabDeckEditor/aOpenCustomFolder"));
    aEditSets->setShortcuts(settingsCache->shortcuts().getShortcut("TabDeckEditor/aEditSets"));
    aEditTokens->setShortcuts(settingsCache->shortcuts().getShortcut("TabDeckEditor/aEditTokens"));
    aResetLayout->setShortcuts(settingsCache->shortcuts().getShortcut("TabDeckEditor/aResetLayout"));
    aClearFilterAll->setShortcuts(settingsCache->shortcuts().getShortcut("TabDeckEditor/aClearFilterAll"));
    aClearFilterOne->setShortcuts(settingsCache->shortcuts().getShortcut("TabDeckEditor/aClearFilterOne"));
    aSaveDeckToClipboard->setShortcuts(settingsCache->shortcuts().getShortcut("TabDeckEditor/aSaveDeckToClipboard"));
    aClearFilterOne->setShortcuts(settingsCache->shortcuts().getShortcut("TabDeckEditor/aClearFilterOne"));
    aClose->setShortcuts(settingsCache->shortcuts().getShortcut("TabDeckEditor/aClose"));
    aRemoveCard->setShortcuts(settingsCache->shortcuts().getShortcut("TabDeckEditor/aRemoveCard"));
    aIncrement->setShortcuts(settingsCache->shortcuts().getShortcut("TabDeckEditor/aIncrement"));
    aDecrement->setShortcuts(settingsCache->shortcuts().getShortcut("TabDeckEditor/aDecrement"));
}

void TabDeckEditor::loadLayout()
{
    MainWindow->restoreState(settingsCache->layouts().getDeckEditorLayoutState());
    MainWindow->restoreGeometry(settingsCache->layouts().getDeckEditorGeometry());

    btnCard->setChecked(!cardInfoDock->isHidden());
    btnFilter->setChecked(!filterDock->isHidden());
    btnDeck->setChecked(!deckDock->isHidden());

    cardInfoDock->setMinimumSize(settingsCache->layouts().getDeckEditorCardSize());
    cardInfoDock->setMaximumSize(settingsCache->layouts().getDeckEditorCardSize());

    filterDock->setMinimumSize(settingsCache->layouts().getDeckEditorFilterSize());
    filterDock->setMaximumSize(settingsCache->layouts().getDeckEditorFilterSize());

    deckDock->setMinimumSize(settingsCache->layouts().getDeckEditorDeckSize());
    deckDock->setMaximumSize(settingsCache->layouts().getDeckEditorDeckSize());

    QTimer::singleShot(100, this, SLOT(freeDocksSize()));
}

TabDeckEditor::TabDeckEditor(TabSupervisor *_tabSupervisor, QWidget *parent)
    : Tab(_tabSupervisor, parent), modified(false)
{
    MainWindow = new QMainWindow;
    MainWindow->setObjectName("MainWindow");

    createMenus();

    createCentralFrame();

    createDeckDock();
    createCardInfoDock();
    createFiltersDock();

    restartLayout();

    this->installEventFilter(this);    

    retranslateUi();
    connect(&settingsCache->shortcuts(), SIGNAL(shortCutchanged()),this,SLOT(refreshShortcuts()));
    refreshShortcuts();

    QTimer::singleShot(0, this, SLOT(checkFirstRunDetected()));
    QTimer::singleShot(0, this, SLOT(loadLayout()));
}

TabDeckEditor::~TabDeckEditor()
{
    emit deckEditorClosing(this);
}

void TabDeckEditor::retranslateUi()
{
    cardInfo->retranslateUi();

    aClearFilterAll->setText(tr("&Clear all filters"));
    aClearFilterOne->setText(tr("Delete selected"));
    
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
    aOpenCustomsetsFolder->setText(tr("Open custom sets folder"));    
    aClose->setText(tr("&Close"));
    
    aAddCard->setText(tr("Add card to &maindeck"));
    aAddCardToSideboard->setText(tr("Add card to &sideboard"));

    aRemoveCard->setText(tr("&Remove row"));

    aIncrement->setText(tr("&Increment number"));

    aDecrement->setText(tr("&Decrement number"));
    
    deckMenu->setTitle(tr("&Deck Editor"));
    dbMenu->setTitle(tr("C&ard Database"));
    
    aEditSets->setText(tr("&Edit sets..."));
    aEditTokens->setText(tr("Edit &tokens..."));

    btnCard->setToolTip(tr("Show/Hide card information"));
    btnDeck->setToolTip(tr("Show/Hide deck"));
    btnFilter->setToolTip(tr("Show/Hide filters"));
    aResetLayout->setText(tr("Reset layout"));
    cardInfoDock->setWindowTitle(tr("Card Info"));
    deckDock->setWindowTitle(tr("Deck"));
    filterDock->setWindowTitle(tr("Filters"));
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

void TabDeckEditor::actOpenCustomsetsFolder() {
#if QT_VERSION < 0x050000
    QString dataDir = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
#else
    QString dataDir = QStandardPaths::standardLocations(QStandardPaths::DataLocation).first();
#endif

#if defined(Q_OS_MAC)

    QStringList scriptArgs;
    scriptArgs << QLatin1String("-e");
    scriptArgs << QString::fromLatin1("tell application \"Finder\" to open POSIX file \"%1\"").arg(dataDir + "/customsets/");
    scriptArgs << QLatin1String("-e");
    scriptArgs << QLatin1String("tell application \"Finder\" to activate");

    QProcess::execute("/usr/bin/osascript", scriptArgs);
#endif
#if defined(Q_OS_WIN)
    QStringList args;
    dataDir.append("/customsets");
    args << QDir::toNativeSeparators(dataDir);
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

void TabDeckEditor::actClearFilterAll()
{
    databaseDisplayModel->clearFilterAll();
}

void TabDeckEditor::actClearFilterOne()
{
    QModelIndexList selIndexes = filterView->selectionModel()->selectedIndexes();
    foreach(QModelIndex idx, selIndexes)
        filterModel->removeRow(idx.row(), idx.parent());
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
    searchEdit->setSelection(0, searchEdit->text().length());
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
    if(QApplication::keyboardModifiers() & Qt::ControlModifier)
        actAddCardToSideboard();
    else
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

bool TabDeckEditor::eventFilter(QObject * o, QEvent * e)
{
    if(e->type() == QEvent::Close)
    {
        if(o == cardInfoDock)
            btnCard->setChecked(false);
        else if(o == deckDock)
            btnDeck->setChecked(false);
        else if(o == filterDock)
            btnFilter->setChecked(false);            
    }   
    if( o == this && e->type() == QEvent::Hide){
        settingsCache->layouts().setDeckEditorLayoutState(MainWindow->saveState());
        settingsCache->layouts().setDeckEditorGeometry(MainWindow->saveGeometry());
        settingsCache->layouts().setDeckEditorCardSize(cardInfoDock->size());
        settingsCache->layouts().setDeckEditorFilterSize(filterDock->size());
        settingsCache->layouts().setDeckEditorDeckSize(deckDock->size());
    }
    return false;
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

    PictureLoader::cacheCardPixmaps(db->getCards(deckModel->getDeckList()->getCardList()));
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
        QMessageBox::information(this, tr("Welcome"), tr("Hi! It seems like you're running this version of Cockatrice for the first time.\nAll the sets in the card database have been enabled.\nRead more about changing the set order or disabling specific sets and consequent effects in the \"Edit Sets\" window."));
        actEditSets();
    }
}
