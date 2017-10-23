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
#include "carddatabase.h"
#include "pictureloader.h"
#include "carddatabasemodel.h"
#include "decklistmodel.h"
#include "dlg_load_deck_from_clipboard.h"
#include "main.h"
#include "settingscache.h"
#include "priceupdater.h"
#include "tab_supervisor.h"
#include "deckstats_interface.h"
#include "tappedout_interface.h"
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
    deckView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
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

    QVBoxLayout *rightFrame = new QVBoxLayout;
    rightFrame->setObjectName("rightFrame");
    rightFrame->addLayout(grid);
    rightFrame->addWidget(deckView, 10);

    deckDock = new QDockWidget(this);
    deckDock->setObjectName("deckDock");

    deckDock->setMinimumSize(QSize(200, 41));
    deckDock->setAllowedAreas(Qt::LeftDockWidgetArea|Qt::RightDockWidgetArea);
    deckDock->setFeatures(QDockWidget::DockWidgetClosable|QDockWidget::DockWidgetFloatable|QDockWidget::DockWidgetMovable);
    QWidget *deckDockContents = new QWidget();
    deckDockContents->setObjectName("deckDockContents");
    deckDockContents->setLayout(rightFrame);
    deckDock->setWidget(deckDockContents);

    deckDock->installEventFilter(this);
    connect(deckDock, SIGNAL(topLevelChanged(bool)), this, SLOT(dockTopLevelChanged(bool)));
}

void TabDeckEditor::createCardInfoDock()
{
    cardInfo = new CardFrame();
    cardInfo->setObjectName("cardInfo");
    QVBoxLayout *cardInfoFrame = new QVBoxLayout;
    cardInfoFrame->setObjectName("cardInfoFrame");
    cardInfoFrame->addWidget(cardInfo);

    cardInfoDock = new QDockWidget(this);
    cardInfoDock->setObjectName("cardInfoDock");    

    cardInfoDock->setMinimumSize(QSize(200, 41));
    cardInfoDock->setAllowedAreas(Qt::LeftDockWidgetArea|Qt::RightDockWidgetArea);
    cardInfoDock->setFeatures(QDockWidget::DockWidgetClosable|QDockWidget::DockWidgetFloatable|QDockWidget::DockWidgetMovable);
    QWidget *cardInfoDockContents = new QWidget();
    cardInfoDockContents->setObjectName("cardInfoDockContents");
    cardInfoDockContents->setLayout(cardInfoFrame);
    cardInfoDock->setWidget(cardInfoDockContents);

    cardInfoDock->installEventFilter(this);    
    connect(cardInfoDock, SIGNAL(topLevelChanged(bool)), this, SLOT(dockTopLevelChanged(bool)));
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

    filterDock = new QDockWidget(this);
    filterDock->setObjectName("filterDock");

    filterDock->setFeatures(QDockWidget::DockWidgetClosable|QDockWidget::DockWidgetFloatable|QDockWidget::DockWidgetMovable);
    QWidget *filterDockContents = new QWidget(this);
    filterDockContents->setObjectName("filterDockContents");
    filterDockContents->setLayout(filterFrame);
    filterDock->setWidget(filterDockContents);    

    filterDock->installEventFilter(this);
    connect(filterDock, SIGNAL(topLevelChanged(bool)), this, SLOT(dockTopLevelChanged(bool)));
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

    aLoadDeckFromClipboard = new QAction(QString(), this);
    connect(aLoadDeckFromClipboard, SIGNAL(triggered()), this, SLOT(actLoadDeckFromClipboard()));

    aSaveDeckToClipboard = new QAction(QString(), this);
    connect(aSaveDeckToClipboard, SIGNAL(triggered()), this, SLOT(actSaveDeckToClipboard()));

    aPrintDeck = new QAction(QString(), this);
    connect(aPrintDeck, SIGNAL(triggered()), this, SLOT(actPrintDeck()));

    aAnalyzeDeckDeckstats = new QAction(QString(), this);
    connect(aAnalyzeDeckDeckstats, SIGNAL(triggered()), this, SLOT(actAnalyzeDeckDeckstats()));

    aAnalyzeDeckTappedout = new QAction(QString(), this);
    connect(aAnalyzeDeckTappedout, SIGNAL(triggered()), this, SLOT(actAnalyzeDeckTappedout()));

    analyzeDeckMenu = new QMenu(this);
    analyzeDeckMenu->addAction(aAnalyzeDeckDeckstats);
    analyzeDeckMenu->addAction(aAnalyzeDeckTappedout);

    aClose = new QAction(QString(), this);
    connect(aClose, SIGNAL(triggered()), this, SLOT(closeRequest()));

    aClearFilterAll = new QAction(QString(), this);
    aClearFilterAll->setIcon(QPixmap("theme:icons/clearsearch"));
    connect(aClearFilterAll, SIGNAL(triggered()), this, SLOT(actClearFilterAll()));

    aClearFilterOne = new QAction(QString(), this);
    aClearFilterOne->setIcon(QPixmap("theme:icons/decrement"));
    connect(aClearFilterOne, SIGNAL(triggered()), this, SLOT(actClearFilterOne()));

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
    deckMenu->addMenu(analyzeDeckMenu);
    deckMenu->addSeparator();
    deckMenu->addAction(aClearFilterOne);
    deckMenu->addAction(aClearFilterAll);
    deckMenu->addSeparator();
    deckMenu->addAction(aClose);
    addTabMenu(deckMenu);

    viewMenu = new QMenu(this);

    cardInfoDockMenu = viewMenu->addMenu(QString());
    deckDockMenu = viewMenu->addMenu(QString());
    filterDockMenu = viewMenu->addMenu(QString());

    aCardInfoDockVisible = cardInfoDockMenu->addAction(QString());
    aCardInfoDockVisible->setCheckable(true);
    connect(aCardInfoDockVisible,SIGNAL(triggered()),this,SLOT(dockVisibleTriggered()));
    aCardInfoDockFloating = cardInfoDockMenu->addAction(QString());
    aCardInfoDockFloating->setCheckable(true);
    connect(aCardInfoDockFloating,SIGNAL(triggered()),this,SLOT(dockFloatingTriggered()));

    aDeckDockVisible = deckDockMenu->addAction(QString());
    aDeckDockVisible->setCheckable(true);
    connect(aDeckDockVisible,SIGNAL(triggered()),this,SLOT(dockVisibleTriggered()));
    aDeckDockFloating = deckDockMenu->addAction(QString());
    aDeckDockFloating->setCheckable(true);
    connect(aDeckDockFloating,SIGNAL(triggered()),this,SLOT(dockFloatingTriggered()));

    aFilterDockVisible = filterDockMenu->addAction(QString());
    aFilterDockVisible->setCheckable(true);
    connect(aFilterDockVisible,SIGNAL(triggered()),this,SLOT(dockVisibleTriggered()));
    aFilterDockFloating = filterDockMenu->addAction(QString());
    aFilterDockFloating->setCheckable(true);
    connect(aFilterDockFloating,SIGNAL(triggered()),this,SLOT(dockFloatingTriggered()));

    viewMenu->addSeparator();

    aResetLayout = viewMenu->addAction(QString());
    connect(aResetLayout,SIGNAL(triggered()),this,SLOT(restartLayout()));
    viewMenu->addAction(aResetLayout);

    addTabMenu(viewMenu);
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
    connect(databaseView->selectionModel(), SIGNAL(currentRowChanged(const QModelIndex &, const QModelIndex &)), this, SLOT(updateCardInfoLeft(const QModelIndex &, const QModelIndex &)));
    connect(databaseView, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(actAddCard()));

    QByteArray dbHeaderState = settingsCache->layouts().getDeckEditorDbHeaderState();
    if(dbHeaderState.isNull())
    {
        // first run
        databaseView->setColumnWidth(0, 200);
    } else {
        databaseView->header()->restoreState(dbHeaderState);
    }
    connect(databaseView->header(), SIGNAL(geometriesChanged()), this, SLOT(saveDbHeaderState()));

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

    centralFrame = new QVBoxLayout;
    centralFrame->setObjectName("centralFrame");
    centralFrame->addLayout(searchLayout);
    centralFrame->addWidget(databaseView);

    centralWidget = new QWidget(this);
    centralWidget->setObjectName("centralWidget");
    centralWidget->setLayout(centralFrame);
    setCentralWidget(centralWidget);
    setDockOptions(QMainWindow::AnimatedDocks|QMainWindow::AllowNestedDocks|QMainWindow::AllowTabbedDocks);
}

void TabDeckEditor::restartLayout()
{
    deckDock->setVisible(true);
    cardInfoDock->setVisible(true);
    filterDock->setVisible(true);

    deckDock->setFloating(false);
    cardInfoDock->setFloating(false);
    filterDock->setFloating(false);

    aCardInfoDockVisible->setChecked(true);
    aDeckDockVisible->setChecked(true);
    aFilterDockVisible->setChecked(true);

    aCardInfoDockFloating->setChecked(false);
    aDeckDockFloating->setChecked(false);
    aFilterDockFloating->setChecked(false);

    addDockWidget(static_cast<Qt::DockWidgetArea>(2), deckDock);
    addDockWidget(static_cast<Qt::DockWidgetArea>(2), cardInfoDock);
    addDockWidget(static_cast<Qt::DockWidgetArea>(2), filterDock);

    splitDockWidget(cardInfoDock, deckDock, Qt::Horizontal);
    splitDockWidget(cardInfoDock, filterDock, Qt::Vertical);

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
    aAnalyzeDeckDeckstats->setShortcuts(settingsCache->shortcuts().getShortcut("TabDeckEditor/aAnalyzeDeck"));
    aClose->setShortcuts(settingsCache->shortcuts().getShortcut("TabDeckEditor/aClose"));
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
    restoreState(settingsCache->layouts().getDeckEditorLayoutState());
    restoreGeometry(settingsCache->layouts().getDeckEditorGeometry());

    aCardInfoDockVisible->setChecked(cardInfoDock->isVisible());
    aFilterDockVisible->setChecked(filterDock->isVisible());
    aDeckDockVisible->setChecked(deckDock->isVisible());

    aCardInfoDockFloating->setEnabled(aCardInfoDockVisible->isChecked());
    aDeckDockFloating->setEnabled(aDeckDockVisible->isChecked());
    aFilterDockFloating->setEnabled(aFilterDockVisible->isChecked());

    aCardInfoDockFloating->setChecked(cardInfoDock->isFloating());
    aFilterDockFloating->setChecked(filterDock->isFloating());
    aDeckDockFloating->setChecked(deckDock->isFloating());

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
    setObjectName("TabDeckEditor");

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

    analyzeDeckMenu->setTitle(tr("&Analyze deck online"));
    aAnalyzeDeckDeckstats->setText("deckstats.net");
    aAnalyzeDeckTappedout->setText("tappedout.net");

    aClose->setText(tr("&Close"));
    
    aAddCard->setText(tr("Add card to &maindeck"));
    aAddCardToSideboard->setText(tr("Add card to &sideboard"));

    aRemoveCard->setText(tr("&Remove row"));

    aIncrement->setText(tr("&Increment number"));

    aDecrement->setText(tr("&Decrement number"));
    
    deckMenu->setTitle(tr("&Deck Editor"));

    cardInfoDock->setWindowTitle(tr("Card Info"));
    deckDock->setWindowTitle(tr("Deck"));
    filterDock->setWindowTitle(tr("Filters"));

    viewMenu->setTitle(tr("&View"));
    cardInfoDockMenu->setTitle(tr("Card Info"));
    deckDockMenu->setTitle(tr("Deck"));
    filterDockMenu->setTitle(tr("Filters"));

    aCardInfoDockVisible->setText(tr("Visible"));
    aCardInfoDockFloating->setText(tr("Floating"));

    aDeckDockVisible->setText(tr("Visible"));
    aDeckDockFloating->setText(tr("Floating"));

    aFilterDockVisible->setText(tr("Visible"));
    aFilterDockFloating->setText(tr("Floating"));

    aResetLayout->setText(tr("Reset layout"));
}

QString TabDeckEditor::getTabText() const
{
    QString result = tr("Deck: %1").arg(nameEdit->text().simplified());
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

void TabDeckEditor::actAnalyzeDeckDeckstats()
{
    DeckStatsInterface *interface = new DeckStatsInterface(
        *databaseModel->getDatabase(),
        this
    ); // it deletes itself when done
    interface->analyzeDeck(deckModel->getDeckList());
}

void TabDeckEditor::actAnalyzeDeckTappedout()
{
    TappedOutInterface *interface = new TappedOutInterface(
        *databaseModel->getDatabase(),
        this
    ); // it deletes itself when done
    interface->analyzeDeck(deckModel->getDeckList());
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

// Method uses to sync docks state with menu items state
bool TabDeckEditor::eventFilter(QObject * o, QEvent * e)
{
    if(e->type() == QEvent::Close)
    {
        if(o == cardInfoDock)
        {
            aCardInfoDockVisible->setChecked(false);
            aCardInfoDockFloating->setEnabled(false);
        } else if(o == deckDock) {
            aDeckDockVisible->setChecked(false);
            aDeckDockFloating->setEnabled(false);
        } else if(o == filterDock) {
            aFilterDockVisible->setChecked(false);
            aFilterDockFloating->setEnabled(false);
        }
    }   
    if( o == this && e->type() == QEvent::Hide){
        settingsCache->layouts().setDeckEditorLayoutState(saveState());
        settingsCache->layouts().setDeckEditorGeometry(saveGeometry());
        settingsCache->layouts().setDeckEditorCardSize(cardInfoDock->size());
        settingsCache->layouts().setDeckEditorFilterSize(filterDock->size());
        settingsCache->layouts().setDeckEditorDeckSize(deckDock->size());
    }
    return false;
}

void TabDeckEditor::dockVisibleTriggered()
{
    QObject *o = sender();
    if(o == aCardInfoDockVisible)
    {
        cardInfoDock->setVisible(aCardInfoDockVisible->isChecked());
        aCardInfoDockFloating->setEnabled(aCardInfoDockVisible->isChecked());
        return;
    }

    if(o == aDeckDockVisible)
    {
        deckDock->setVisible(aDeckDockVisible->isChecked());
        aDeckDockFloating->setEnabled(aDeckDockVisible->isChecked());
        return;
    }

    if(o == aFilterDockVisible)
    {
        filterDock->setVisible(aFilterDockVisible->isChecked());
        aFilterDockFloating->setEnabled(aFilterDockVisible->isChecked());
        return;
    }
}

void TabDeckEditor::dockFloatingTriggered()
{
    QObject *o = sender();
    if(o == aCardInfoDockFloating)
    {
        cardInfoDock->setFloating(aCardInfoDockFloating->isChecked());
        return;
    }

    if(o == aDeckDockFloating)
    {
        deckDock->setFloating(aDeckDockFloating->isChecked());
        return;
    }

    if(o == aFilterDockFloating)
    {
        filterDock->setFloating(aFilterDockFloating->isChecked());
        return;
    }
}

void TabDeckEditor::dockTopLevelChanged(bool topLevel)
{
    QObject *o = sender();
    if(o == cardInfoDock)
    {
        aCardInfoDockFloating->setChecked(topLevel);
        return;
    }

    if(o == deckDock)
    {
        aDeckDockFloating->setChecked(topLevel);
        return;
    }

    if(o == filterDock)
    {
        aFilterDockFloating->setChecked(topLevel);
        return;
    }
}

void TabDeckEditor::saveDbHeaderState()
{
    settingsCache->layouts().setDeckEditorDbHeaderState(databaseView->header()->saveState());
}
