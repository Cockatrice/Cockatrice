#include "tab_deck_editor.h"

#include "../../client/game_logic/abstract_client.h"
#include "../../client/tapped_out_interface.h"
#include "../../client/ui/widgets/cards/card_info_frame_widget.h"
#include "../../dialogs/dlg_load_deck.h"
#include "../../dialogs/dlg_load_deck_from_clipboard.h"
#include "../../game/cards/card_database_manager.h"
#include "../../game/cards/card_database_model.h"
#include "../../game/filters/filter_builder.h"
#include "../../game/filters/filter_tree_model.h"
#include "../../server/pending_command.h"
#include "../../settings/cache_settings.h"
#include "../ui/pixel_map_generator.h"
#include "tab_supervisor.h"
#include "trice_limits.h"

#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QComboBox>
#include <QDebug>
#include <QDir>
#include <QDockWidget>
#include <QFileDialog>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMenuBar>
#include <QMessageBox>
#include <QPrintPreviewDialog>
#include <QProcessEnvironment>
#include <QSplitter>
#include <QTextEdit>
#include <QTimer>
#include <QToolButton>
#include <QTreeView>
#include <QVBoxLayout>

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
    deckView->setContextMenuPolicy(Qt::CustomContextMenu);
    deckView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    connect(deckView->selectionModel(), SIGNAL(currentRowChanged(const QModelIndex &, const QModelIndex &)), this,
            SLOT(updateCardInfoRight(const QModelIndex &, const QModelIndex &)));
    connect(deckView->selectionModel(), SIGNAL(currentRowChanged(const QModelIndex &, const QModelIndex &)), this,
            SLOT(updatePrintingSelectorDeckView(const QModelIndex &, const QModelIndex &)));
    connect(deckView, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(actSwapCard()));
    connect(deckView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(decklistCustomMenu(QPoint)));
    connect(&deckViewKeySignals, SIGNAL(onShiftS()), this, SLOT(actSwapCard()));
    connect(&deckViewKeySignals, SIGNAL(onEnter()), this, SLOT(actIncrement()));
    connect(&deckViewKeySignals, SIGNAL(onCtrlAltEqual()), this, SLOT(actIncrement()));
    connect(&deckViewKeySignals, SIGNAL(onCtrlAltMinus()), this, SLOT(actDecrement()));
    connect(&deckViewKeySignals, SIGNAL(onShiftRight()), this, SLOT(actIncrement()));
    connect(&deckViewKeySignals, SIGNAL(onShiftLeft()), this, SLOT(actDecrement()));
    connect(&deckViewKeySignals, SIGNAL(onDelete()), this, SLOT(actRemoveCard()));

    nameLabel = new QLabel();
    nameLabel->setObjectName("nameLabel");
    nameEdit = new LineEditUnfocusable;
    nameEdit->setMaxLength(MAX_NAME_LENGTH);
    nameEdit->setObjectName("nameEdit");
    nameLabel->setBuddy(nameEdit);
    connect(nameEdit, SIGNAL(textChanged(const QString &)), this, SLOT(updateName(const QString &)));
    commentsLabel = new QLabel();
    commentsLabel->setObjectName("commentsLabel");
    commentsEdit = new QTextEdit;
    commentsEdit->setAcceptRichText(false);
    commentsEdit->setMinimumHeight(nameEdit->minimumSizeHint().height());
    commentsEdit->setObjectName("commentsEdit");
    commentsLabel->setBuddy(commentsEdit);
    connect(commentsEdit, SIGNAL(textChanged()), this, SLOT(updateComments()));

    bannerCardLabel = new QLabel();
    bannerCardLabel->setObjectName("bannerCardLabel");
    bannerCardLabel->setText(tr("Banner Card"));
    bannerCardComboBox = new QComboBox(this);
    connect(deckModel, &DeckListModel::dataChanged, this, [this]() {
        // Delay the update to avoid race conditions
        QTimer::singleShot(100, this, &TabDeckEditor::updateBannerCardComboBox);
    });
    connect(bannerCardComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &TabDeckEditor::setBannerCard);

    deckTagsDisplayWidget = new DeckPreviewDeckTagsDisplayWidget(this, deckModel->getDeckList());

    aIncrement = new QAction(QString(), this);
    aIncrement->setIcon(QPixmap("theme:icons/increment"));
    connect(aIncrement, SIGNAL(triggered()), this, SLOT(actIncrement()));
    auto *tbIncrement = new QToolButton(this);
    tbIncrement->setDefaultAction(aIncrement);

    aDecrement = new QAction(QString(), this);
    aDecrement->setIcon(QPixmap("theme:icons/decrement"));
    connect(aDecrement, SIGNAL(triggered()), this, SLOT(actDecrement()));
    auto *tbDecrement = new QToolButton(this);
    tbDecrement->setDefaultAction(aDecrement);

    aRemoveCard = new QAction(QString(), this);
    aRemoveCard->setIcon(QPixmap("theme:icons/remove_row"));
    connect(aRemoveCard, SIGNAL(triggered()), this, SLOT(actRemoveCard()));
    auto *tbRemoveCard = new QToolButton(this);
    tbRemoveCard->setDefaultAction(aRemoveCard);

    aSwapCard = new QAction(QString(), this);
    aSwapCard->setIcon(QPixmap("theme:icons/swap"));
    connect(aSwapCard, SIGNAL(triggered()), this, SLOT(actSwapCard()));
    auto *tbSwapCard = new QToolButton(this);
    tbSwapCard->setDefaultAction(aSwapCard);

    auto *upperLayout = new QGridLayout;
    upperLayout->setObjectName("upperLayout");
    upperLayout->addWidget(nameLabel, 0, 0);
    upperLayout->addWidget(nameEdit, 0, 1);

    upperLayout->addWidget(commentsLabel, 1, 0);
    upperLayout->addWidget(commentsEdit, 1, 1);

    upperLayout->addWidget(bannerCardLabel, 2, 0);
    upperLayout->addWidget(bannerCardComboBox, 2, 1);

    upperLayout->addWidget(deckTagsDisplayWidget, 3, 1);

    hashLabel1 = new QLabel();
    hashLabel1->setObjectName("hashLabel1");
    auto *hashSizePolicy = new QSizePolicy();
    hashSizePolicy->setHorizontalPolicy(QSizePolicy::Fixed);
    hashLabel1->setSizePolicy(*hashSizePolicy);
    hashLabel = new LineEditUnfocusable;
    hashLabel->setObjectName("hashLabel");
    hashLabel->setReadOnly(true);
    hashLabel->setFrame(false);

    auto *lowerLayout = new QGridLayout;
    lowerLayout->setObjectName("lowerLayout");
    lowerLayout->addWidget(hashLabel1, 0, 0);
    lowerLayout->addWidget(hashLabel, 0, 1);
    lowerLayout->addWidget(tbIncrement, 0, 2);
    lowerLayout->addWidget(tbDecrement, 0, 3);
    lowerLayout->addWidget(tbRemoveCard, 0, 4);
    lowerLayout->addWidget(tbSwapCard, 0, 5);
    lowerLayout->addWidget(deckView, 1, 0, 1, 6);

    // Create widgets for both layouts to make splitter work correctly
    auto *topWidget = new QWidget;
    topWidget->setLayout(upperLayout);
    auto *bottomWidget = new QWidget;
    bottomWidget->setLayout(lowerLayout);

    auto *split = new QSplitter;
    split->setObjectName("deckSplitter");
    split->setOrientation(Qt::Vertical);
    split->setChildrenCollapsible(true);
    split->addWidget(topWidget);
    split->addWidget(bottomWidget);
    split->setStretchFactor(0, 1);
    split->setStretchFactor(1, 4);

    auto *rightFrame = new QVBoxLayout;
    rightFrame->setObjectName("rightFrame");
    rightFrame->addWidget(split);

    deckDock = new QDockWidget(this);
    deckDock->setObjectName("deckDock");

    deckDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    deckDock->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetFloatable |
                          QDockWidget::DockWidgetMovable);
    auto *deckDockContents = new QWidget();
    deckDockContents->setObjectName("deckDockContents");
    deckDockContents->setLayout(rightFrame);
    deckDock->setWidget(deckDockContents);

    deckDock->installEventFilter(this);
    connect(deckDock, SIGNAL(topLevelChanged(bool)), this, SLOT(dockTopLevelChanged(bool)));
}

void TabDeckEditor::createCardInfoDock()
{
    cardInfo = new CardInfoFrameWidget();
    cardInfo->setObjectName("cardInfo");
    auto *cardInfoFrame = new QVBoxLayout;
    cardInfoFrame->setObjectName("cardInfoFrame");
    cardInfoFrame->addWidget(cardInfo);

    cardInfoDock = new QDockWidget(this);
    cardInfoDock->setObjectName("cardInfoDock");

    cardInfoDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    cardInfoDock->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetFloatable |
                              QDockWidget::DockWidgetMovable);
    auto *cardInfoDockContents = new QWidget();
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
    filterView->installEventFilter(&filterViewKeySignals);
    connect(filterModel, SIGNAL(layoutChanged()), filterView, SLOT(expandAll()));
    connect(filterView, SIGNAL(customContextMenuRequested(const QPoint &)), this,
            SLOT(filterViewCustomContextMenu(const QPoint &)));
    connect(&filterViewKeySignals, SIGNAL(onDelete()), this, SLOT(actClearFilterOne()));

    auto *filterBuilder = new FilterBuilder;
    filterBuilder->setObjectName("filterBuilder");
    connect(filterBuilder, SIGNAL(add(const CardFilter *)), filterModel, SLOT(addFilter(const CardFilter *)));

    auto *filterDelOne = new QToolButton();
    filterDelOne->setObjectName("filterDelOne");
    filterDelOne->setDefaultAction(aClearFilterOne);
    filterDelOne->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    auto *filterDelAll = new QToolButton();
    filterDelAll->setObjectName("filterDelAll");
    filterDelAll->setDefaultAction(aClearFilterAll);
    filterDelAll->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    auto *filterLayout = new QGridLayout;
    filterLayout->setObjectName("filterLayout");
    filterLayout->setContentsMargins(0, 0, 0, 0);
    filterLayout->addWidget(filterBuilder, 0, 0, 1, 3);
    filterLayout->addWidget(filterView, 1, 0, 1, 3);
    filterLayout->addWidget(filterDelOne, 2, 0, 1, 1);
    filterLayout->addWidget(filterDelAll, 2, 2, 1, 1);

    filterBox = new QWidget();
    filterBox->setObjectName("filterBox");
    filterBox->setLayout(filterLayout);

    auto *filterFrame = new QVBoxLayout;
    filterFrame->setObjectName("filterFrame");
    filterFrame->addWidget(filterBox);

    filterDock = new QDockWidget(this);
    filterDock->setObjectName("filterDock");

    filterDock->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetFloatable |
                            QDockWidget::DockWidgetMovable);
    auto *filterDockContents = new QWidget(this);
    filterDockContents->setObjectName("filterDockContents");
    filterDockContents->setLayout(filterFrame);
    filterDock->setWidget(filterDockContents);

    filterDock->installEventFilter(this);
    connect(filterDock, SIGNAL(topLevelChanged(bool)), this, SLOT(dockTopLevelChanged(bool)));
}

void TabDeckEditor::createPrintingSelectorDock()
{
    printingSelector = new PrintingSelector(this, this, deckModel, deckView);
    printingSelector->setObjectName("printingSelector");
    auto *printingSelectorFrame = new QVBoxLayout;
    printingSelectorFrame->setObjectName("printingSelectorFrame");
    printingSelectorFrame->addWidget(printingSelector);

    printingSelectorDock = new QDockWidget(this);
    printingSelectorDock->setObjectName("printingSelectorDock");

    printingSelectorDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    printingSelectorDock->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetFloatable |
                                      QDockWidget::DockWidgetMovable);
    auto *printingSelectorDockContents = new QWidget();
    printingSelectorDockContents->setObjectName("printingSelectorDockContents");
    printingSelectorDockContents->setLayout(printingSelectorFrame);
    printingSelectorDock->setWidget(printingSelectorDockContents);

    printingSelectorDock->installEventFilter(this);
    connect(printingSelectorDock, SIGNAL(topLevelChanged(bool)), this, SLOT(dockTopLevelChanged(bool)));

    addDockWidget(Qt::RightDockWidgetArea, printingSelectorDock);
    printingSelectorDock->setFloating(false);
}

void TabDeckEditor::createMenus()
{
    aNewDeck = new QAction(QString(), this);
    connect(aNewDeck, SIGNAL(triggered()), this, SLOT(actNewDeck()));

    aLoadDeck = new QAction(QString(), this);
    connect(aLoadDeck, SIGNAL(triggered()), this, SLOT(actLoadDeck()));

    loadRecentDeckMenu = new QMenu(this);
    connect(&SettingsCache::instance().recents(), &RecentsSettings::recentlyOpenedDeckPathsChanged, this,
            &TabDeckEditor::updateRecentlyOpened);

    aClearRecents = new QAction(QString(), this);
    connect(aClearRecents, &QAction::triggered, this, &TabDeckEditor::actClearRecents);

    updateRecentlyOpened();

    aSaveDeck = new QAction(QString(), this);
    connect(aSaveDeck, SIGNAL(triggered()), this, SLOT(actSaveDeck()));

    aSaveDeckAs = new QAction(QString(), this);
    connect(aSaveDeckAs, SIGNAL(triggered()), this, SLOT(actSaveDeckAs()));

    aLoadDeckFromClipboard = new QAction(QString(), this);
    connect(aLoadDeckFromClipboard, SIGNAL(triggered()), this, SLOT(actLoadDeckFromClipboard()));

    aSaveDeckToClipboard = new QAction(QString(), this);
    connect(aSaveDeckToClipboard, SIGNAL(triggered()), this, SLOT(actSaveDeckToClipboard()));

    aSaveDeckToClipboardNoSetNameAndNumber = new QAction(QString(), this);
    connect(aSaveDeckToClipboardNoSetNameAndNumber, SIGNAL(triggered()), this,
            SLOT(actSaveDeckToClipboardNoSetNameAndNumber()));

    aSaveDeckToClipboardRaw = new QAction(QString(), this);
    connect(aSaveDeckToClipboardRaw, SIGNAL(triggered()), this, SLOT(actSaveDeckToClipboardRaw()));

    aSaveDeckToClipboardRawNoSetNameAndNumber = new QAction(QString(), this);
    connect(aSaveDeckToClipboardRawNoSetNameAndNumber, SIGNAL(triggered()), this,
            SLOT(actSaveDeckToClipboardRawNoSetNameAndNumber()));

    aPrintDeck = new QAction(QString(), this);
    connect(aPrintDeck, SIGNAL(triggered()), this, SLOT(actPrintDeck()));

    aExportDeckDecklist = new QAction(QString(), this);
    connect(aExportDeckDecklist, SIGNAL(triggered()), this, SLOT(actExportDeckDecklist()));

    aAnalyzeDeckDeckstats = new QAction(QString(), this);
    connect(aAnalyzeDeckDeckstats, SIGNAL(triggered()), this, SLOT(actAnalyzeDeckDeckstats()));

    aAnalyzeDeckTappedout = new QAction(QString(), this);
    connect(aAnalyzeDeckTappedout, SIGNAL(triggered()), this, SLOT(actAnalyzeDeckTappedout()));

    analyzeDeckMenu = new QMenu(this);
    analyzeDeckMenu->addAction(aExportDeckDecklist);
    analyzeDeckMenu->addAction(aAnalyzeDeckDeckstats);
    analyzeDeckMenu->addAction(aAnalyzeDeckTappedout);

    aClose = new QAction(QString(), this);
    connect(aClose, &QAction::triggered, this, [this] { closeRequest(); });

    aClearFilterAll = new QAction(QString(), this);
    aClearFilterAll->setIcon(QPixmap("theme:icons/clearsearch"));
    connect(aClearFilterAll, SIGNAL(triggered()), this, SLOT(actClearFilterAll()));

    aClearFilterOne = new QAction(QString(), this);
    aClearFilterOne->setIcon(QPixmap("theme:icons/decrement"));
    connect(aClearFilterOne, SIGNAL(triggered()), this, SLOT(actClearFilterOne()));

    saveDeckToClipboardMenu = new QMenu(this);
    saveDeckToClipboardMenu->addAction(aSaveDeckToClipboard);
    saveDeckToClipboardMenu->addAction(aSaveDeckToClipboardNoSetNameAndNumber);
    saveDeckToClipboardMenu->addAction(aSaveDeckToClipboardRaw);
    saveDeckToClipboardMenu->addAction(aSaveDeckToClipboardRawNoSetNameAndNumber);

    deckMenu = new QMenu(this);
    deckMenu->addAction(aNewDeck);
    deckMenu->addAction(aLoadDeck);
    deckMenu->addMenu(loadRecentDeckMenu);
    deckMenu->addAction(aSaveDeck);
    deckMenu->addAction(aSaveDeckAs);
    deckMenu->addSeparator();
    deckMenu->addAction(aLoadDeckFromClipboard);
    deckMenu->addMenu(saveDeckToClipboardMenu);
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
    printingSelectorDockMenu = viewMenu->addMenu(QString());

    aCardInfoDockVisible = cardInfoDockMenu->addAction(QString());
    aCardInfoDockVisible->setCheckable(true);
    connect(aCardInfoDockVisible, SIGNAL(triggered()), this, SLOT(dockVisibleTriggered()));
    aCardInfoDockFloating = cardInfoDockMenu->addAction(QString());
    aCardInfoDockFloating->setCheckable(true);
    connect(aCardInfoDockFloating, SIGNAL(triggered()), this, SLOT(dockFloatingTriggered()));

    aDeckDockVisible = deckDockMenu->addAction(QString());
    aDeckDockVisible->setCheckable(true);
    connect(aDeckDockVisible, SIGNAL(triggered()), this, SLOT(dockVisibleTriggered()));
    aDeckDockFloating = deckDockMenu->addAction(QString());
    aDeckDockFloating->setCheckable(true);
    connect(aDeckDockFloating, SIGNAL(triggered()), this, SLOT(dockFloatingTriggered()));

    aFilterDockVisible = filterDockMenu->addAction(QString());
    aFilterDockVisible->setCheckable(true);
    connect(aFilterDockVisible, SIGNAL(triggered()), this, SLOT(dockVisibleTriggered()));
    aFilterDockFloating = filterDockMenu->addAction(QString());
    aFilterDockFloating->setCheckable(true);
    connect(aFilterDockFloating, SIGNAL(triggered()), this, SLOT(dockFloatingTriggered()));

    aPrintingSelectorDockVisible = printingSelectorDockMenu->addAction(QString());
    aPrintingSelectorDockVisible->setCheckable(true);
    connect(aPrintingSelectorDockVisible, SIGNAL(triggered()), this, SLOT(dockVisibleTriggered()));
    aPrintingSelectorDockFloating = printingSelectorDockMenu->addAction(QString());
    aPrintingSelectorDockFloating->setCheckable(true);
    connect(aPrintingSelectorDockFloating, SIGNAL(triggered()), this, SLOT(dockFloatingTriggered()));

    viewMenu->addSeparator();

    aResetLayout = viewMenu->addAction(QString());
    connect(aResetLayout, SIGNAL(triggered()), this, SLOT(restartLayout()));
    viewMenu->addAction(aResetLayout);

    setSaveStatus(false);

    addTabMenu(viewMenu);
}

void TabDeckEditor::createCentralFrame()
{
    searchEdit = new SearchLineEdit;
    searchEdit->setObjectName("searchEdit");
    searchEdit->setPlaceholderText(tr("Search by card name (or search expressions)"));
    searchEdit->setClearButtonEnabled(true);
    searchEdit->addAction(loadColorAdjustedPixmap("theme:icons/search"), QLineEdit::LeadingPosition);
    auto help = searchEdit->addAction(QPixmap("theme:icons/info"), QLineEdit::TrailingPosition);
    searchEdit->installEventFilter(&searchKeySignals);

    setFocusProxy(searchEdit);
    setFocusPolicy(Qt::ClickFocus);

    searchKeySignals.setObjectName("searchKeySignals");
    connect(searchEdit, SIGNAL(textChanged(const QString &)), this, SLOT(updateSearch(const QString &)));
    connect(&searchKeySignals, SIGNAL(onEnter()), this, SLOT(actAddCard()));
    connect(&searchKeySignals, SIGNAL(onCtrlAltEqual()), this, SLOT(actAddCard()));
    connect(&searchKeySignals, SIGNAL(onCtrlAltRBracket()), this, SLOT(actAddCardToSideboard()));
    connect(&searchKeySignals, SIGNAL(onCtrlAltMinus()), this, SLOT(actDecrementCard()));
    connect(&searchKeySignals, SIGNAL(onCtrlAltLBracket()), this, SLOT(actDecrementCardFromSideboard()));
    connect(&searchKeySignals, SIGNAL(onCtrlAltEnter()), this, SLOT(actAddCardToSideboard()));
    connect(&searchKeySignals, SIGNAL(onCtrlEnter()), this, SLOT(actAddCardToSideboard()));
    connect(&searchKeySignals, SIGNAL(onCtrlC()), this, SLOT(copyDatabaseCellContents()));
    connect(help, &QAction::triggered, this, &TabDeckEditor::showSearchSyntaxHelp);

    databaseModel = new CardDatabaseModel(CardDatabaseManager::getInstance(), true, this);
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
    databaseView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(databaseView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(databaseCustomMenu(QPoint)));
    connect(databaseView->selectionModel(), SIGNAL(currentRowChanged(const QModelIndex &, const QModelIndex &)), this,
            SLOT(updateCardInfoLeft(const QModelIndex &, const QModelIndex &)));
    connect(databaseView->selectionModel(), SIGNAL(currentRowChanged(const QModelIndex &, const QModelIndex &)), this,
            SLOT(updatePrintingSelectorDatabase(const QModelIndex &, const QModelIndex &)));
    connect(databaseView, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(actAddCard()));

    QByteArray dbHeaderState = SettingsCache::instance().layouts().getDeckEditorDbHeaderState();
    if (dbHeaderState.isNull()) {
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
    auto *tbAddCard = new QToolButton(this);
    tbAddCard->setDefaultAction(aAddCard);

    aAddCardToSideboard = new QAction(QString(), this);
    aAddCardToSideboard->setIcon(QPixmap("theme:icons/arrow_right_blue"));
    connect(aAddCardToSideboard, SIGNAL(triggered()), this, SLOT(actAddCardToSideboard()));
    auto *tbAddCardToSideboard = new QToolButton(this);
    tbAddCardToSideboard->setDefaultAction(aAddCardToSideboard);

    searchLayout = new QHBoxLayout;
    searchLayout->setObjectName("searchLayout");
    searchLayout->addWidget(searchEdit);
    searchLayout->addWidget(tbAddCard);
    searchLayout->addWidget(tbAddCardToSideboard);

    centralFrame = new QVBoxLayout;
    centralFrame->setObjectName("centralFrame");
    centralFrame->addLayout(searchLayout);
    centralFrame->addWidget(databaseView);

    centralWidget = new QWidget(this);
    centralWidget->setObjectName("centralWidget");
    centralWidget->setLayout(centralFrame);
    centralWidget->setMaximumSize(900, 5000);
    setCentralWidget(centralWidget);
    setDockOptions(QMainWindow::AnimatedDocks | QMainWindow::AllowNestedDocks | QMainWindow::AllowTabbedDocks);
}

void TabDeckEditor::showPrintingSelector()
{
    printingSelector->setCard(cardInfo->getInfo(), DECK_ZONE_MAIN);
    printingSelector->updateDisplay();
    aPrintingSelectorDockVisible->setChecked(true);
    printingSelectorDock->setVisible(true);
}

void TabDeckEditor::restartLayout()
{
    deckDock->setVisible(true);
    cardInfoDock->setVisible(true);
    filterDock->setVisible(true);
    printingSelectorDock->setVisible(false);

    deckDock->setFloating(false);
    cardInfoDock->setFloating(false);
    filterDock->setFloating(false);
    printingSelectorDock->setFloating(false);

    aCardInfoDockVisible->setChecked(true);
    aDeckDockVisible->setChecked(true);
    aFilterDockVisible->setChecked(true);
    aPrintingSelectorDockVisible->setChecked(false);

    aCardInfoDockFloating->setChecked(false);
    aDeckDockFloating->setChecked(false);
    aFilterDockFloating->setChecked(false);
    aPrintingSelectorDockFloating->setChecked(false);

    addDockWidget(static_cast<Qt::DockWidgetArea>(2), deckDock);
    addDockWidget(static_cast<Qt::DockWidgetArea>(2), cardInfoDock);
    addDockWidget(static_cast<Qt::DockWidgetArea>(2), filterDock);
    addDockWidget(static_cast<Qt::DockWidgetArea>(2), printingSelectorDock);

    splitDockWidget(cardInfoDock, printingSelectorDock, Qt::Horizontal);
    splitDockWidget(printingSelectorDock, deckDock, Qt::Horizontal);
    splitDockWidget(cardInfoDock, printingSelectorDock, Qt::Horizontal);
    splitDockWidget(cardInfoDock, filterDock, Qt::Vertical);

    QTimer::singleShot(100, this, SLOT(freeDocksSize()));
}

void TabDeckEditor::freeDocksSize()
{
    deckDock->setMinimumSize(100, 100);
    deckDock->setMaximumSize(5000, 5000);

    cardInfoDock->setMinimumSize(100, 100);
    cardInfoDock->setMaximumSize(5000, 5000);

    filterDock->setMinimumSize(100, 100);
    filterDock->setMaximumSize(5000, 5000);

    printingSelectorDock->setMinimumSize(525, 100);
    printingSelectorDock->setMaximumSize(5000, 5000);

    centralWidget->setMaximumSize(900, 5000);
}

void TabDeckEditor::refreshShortcuts()
{
    ShortcutsSettings &shortcuts = SettingsCache::instance().shortcuts();
    aNewDeck->setShortcuts(shortcuts.getShortcut("TabDeckEditor/aNewDeck"));
    aLoadDeck->setShortcuts(shortcuts.getShortcut("TabDeckEditor/aLoadDeck"));
    aSaveDeck->setShortcuts(shortcuts.getShortcut("TabDeckEditor/aSaveDeck"));
    aExportDeckDecklist->setShortcuts(shortcuts.getShortcut("TabDeckEditor/aExportDeckDecklist"));
    aSaveDeckAs->setShortcuts(shortcuts.getShortcut("TabDeckEditor/aSaveDeckAs"));
    aLoadDeckFromClipboard->setShortcuts(shortcuts.getShortcut("TabDeckEditor/aLoadDeckFromClipboard"));
    aPrintDeck->setShortcuts(shortcuts.getShortcut("TabDeckEditor/aPrintDeck"));
    aAnalyzeDeckDeckstats->setShortcuts(shortcuts.getShortcut("TabDeckEditor/aAnalyzeDeck"));
    aClose->setShortcuts(shortcuts.getShortcut("TabDeckEditor/aClose"));
    aResetLayout->setShortcuts(shortcuts.getShortcut("TabDeckEditor/aResetLayout"));
    aClearFilterAll->setShortcuts(shortcuts.getShortcut("TabDeckEditor/aClearFilterAll"));
    aClearFilterOne->setShortcuts(shortcuts.getShortcut("TabDeckEditor/aClearFilterOne"));

    aSaveDeckToClipboard->setShortcuts(shortcuts.getShortcut("TabDeckEditor/aSaveDeckToClipboard"));
    aSaveDeckToClipboardRaw->setShortcuts(shortcuts.getShortcut("TabDeckEditor/aSaveDeckToClipboardRaw"));

    aClearFilterOne->setShortcuts(shortcuts.getShortcut("TabDeckEditor/aClearFilterOne"));
    aClose->setShortcuts(shortcuts.getShortcut("TabDeckEditor/aClose"));
    aRemoveCard->setShortcuts(shortcuts.getShortcut("TabDeckEditor/aRemoveCard"));
    aIncrement->setShortcuts(shortcuts.getShortcut("TabDeckEditor/aIncrement"));
    aDecrement->setShortcuts(shortcuts.getShortcut("TabDeckEditor/aDecrement"));
}

void TabDeckEditor::loadLayout()
{
    LayoutsSettings &layouts = SettingsCache::instance().layouts();
    auto &layoutState = layouts.getDeckEditorLayoutState();
    if (layoutState.isNull()) {
        restartLayout();
    } else {
        restoreState(layoutState);
        restoreGeometry(layouts.getDeckEditorGeometry());
    }

    aCardInfoDockVisible->setChecked(!cardInfoDock->isHidden());
    aFilterDockVisible->setChecked(!filterDock->isHidden());
    aDeckDockVisible->setChecked(!deckDock->isHidden());
    aPrintingSelectorDockVisible->setChecked(!printingSelectorDock->isHidden());

    aCardInfoDockFloating->setEnabled(aCardInfoDockVisible->isChecked());
    aDeckDockFloating->setEnabled(aDeckDockVisible->isChecked());
    aFilterDockFloating->setEnabled(aFilterDockVisible->isChecked());
    aPrintingSelectorDockFloating->setEnabled(aPrintingSelectorDockVisible->isChecked());

    aCardInfoDockFloating->setChecked(cardInfoDock->isFloating());
    aFilterDockFloating->setChecked(filterDock->isFloating());
    aDeckDockFloating->setChecked(deckDock->isFloating());
    aPrintingSelectorDockFloating->setChecked(printingSelectorDock->isFloating());

    cardInfoDock->setMinimumSize(layouts.getDeckEditorCardSize());
    cardInfoDock->setMaximumSize(layouts.getDeckEditorCardSize());

    filterDock->setMinimumSize(layouts.getDeckEditorFilterSize());
    filterDock->setMaximumSize(layouts.getDeckEditorFilterSize());

    deckDock->setMinimumSize(layouts.getDeckEditorDeckSize());
    deckDock->setMaximumSize(layouts.getDeckEditorDeckSize());

    printingSelectorDock->setMinimumSize(layouts.getDeckEditorPrintingSelectorSize());
    printingSelectorDock->setMaximumSize(layouts.getDeckEditorPrintingSelectorSize());

    QTimer::singleShot(100, this, SLOT(freeDocksSize()));
}

TabDeckEditor::TabDeckEditor(TabSupervisor *_tabSupervisor) : TabGenericDeckEditor(_tabSupervisor)
{
    setObjectName("TabDeckEditor");

    TabDeckEditor::createMenus();

    TabDeckEditor::createCentralFrame();

    TabDeckEditor::createDeckDock();
    TabDeckEditor::createCardInfoDock();
    TabDeckEditor::createFiltersDock();
    TabDeckEditor::createPrintingSelectorDock();

    this->installEventFilter(this);

    TabDeckEditor::retranslateUi();
    connect(&SettingsCache::instance().shortcuts(), SIGNAL(shortCutChanged()), this, SLOT(refreshShortcuts()));
    TabDeckEditor::refreshShortcuts();

    TabDeckEditor::loadLayout();
}

void TabDeckEditor::retranslateUi()
{
    cardInfo->retranslateUi();

    aClearFilterAll->setText(tr("&Clear all filters"));
    aClearFilterOne->setText(tr("Delete selected"));

    nameLabel->setText(tr("Deck &name:"));
    commentsLabel->setText(tr("&Comments:"));
    hashLabel1->setText(tr("Hash:"));

    aNewDeck->setText(tr("&New deck"));
    aLoadDeck->setText(tr("&Load deck..."));
    loadRecentDeckMenu->setTitle(tr("Load recent deck..."));
    aClearRecents->setText(tr("Clear"));
    aSaveDeck->setText(tr("&Save deck"));
    aSaveDeckAs->setText(tr("Save deck &as..."));
    aLoadDeckFromClipboard->setText(tr("Load deck from cl&ipboard..."));

    saveDeckToClipboardMenu->setTitle(tr("Save deck to clipboard"));
    aSaveDeckToClipboard->setText(tr("Annotated"));
    aSaveDeckToClipboardNoSetNameAndNumber->setText(tr("Annotated (No set name or number)"));
    aSaveDeckToClipboardRaw->setText(tr("Not Annotated"));
    aSaveDeckToClipboardRawNoSetNameAndNumber->setText(tr("Not Annotated (No set name or number)"));

    aPrintDeck->setText(tr("&Print deck..."));

    analyzeDeckMenu->setTitle(tr("&Send deck to online service"));
    aExportDeckDecklist->setText(tr("Create decklist (decklist.org)"));
    aAnalyzeDeckDeckstats->setText(tr("Analyze deck (deckstats.net)"));
    aAnalyzeDeckTappedout->setText(tr("Analyze deck (tappedout.net)"));

    aClose->setText(tr("&Close"));

    aAddCard->setText(tr("Add card to &maindeck"));
    aAddCardToSideboard->setText(tr("Add card to &sideboard"));

    aIncrement->setText(tr("&Increment number"));
    aDecrement->setText(tr("&Decrement number"));
    aRemoveCard->setText(tr("&Remove row"));
    aSwapCard->setText(tr("Swap card to/from sideboard"));

    deckMenu->setTitle(tr("&Deck Editor"));

    cardInfoDock->setWindowTitle(tr("Card Info"));
    deckDock->setWindowTitle(tr("Deck"));
    filterDock->setWindowTitle(tr("Filters"));
    printingSelectorDock->setWindowTitle(tr("Printing Selector"));

    viewMenu->setTitle(tr("&View"));
    cardInfoDockMenu->setTitle(tr("Card Info"));
    deckDockMenu->setTitle(tr("Deck"));
    filterDockMenu->setTitle(tr("Filters"));
    printingSelectorDockMenu->setTitle(tr("Printing"));

    aCardInfoDockVisible->setText(tr("Visible"));
    aCardInfoDockFloating->setText(tr("Floating"));

    aDeckDockVisible->setText(tr("Visible"));
    aDeckDockFloating->setText(tr("Floating"));

    aFilterDockVisible->setText(tr("Visible"));
    aFilterDockFloating->setText(tr("Floating"));

    aPrintingSelectorDockVisible->setText(tr("Visible"));
    aPrintingSelectorDockFloating->setText(tr("Floating"));

    aResetLayout->setText(tr("Reset layout"));
}

QString TabDeckEditor::getTabText() const
{
    QString result = tr("Deck: %1").arg(nameEdit->text().simplified());
    if (modified)
        result.prepend("* ");
    return result;
}

void TabDeckEditor::actNewDeck()
{
    auto deckOpenLocation = confirmOpen(false);

    if (deckOpenLocation == CANCELLED) {
        return;
    }

    if (deckOpenLocation == NEW_TAB) {
        emit openDeckEditor(nullptr);
        return;
    }

    deckModel->cleanList();
    nameEdit->setText(QString());
    commentsEdit->setText(QString());
    hashLabel->setText(QString());
    setModified(false);
    setSaveStatus(false);
}

void TabDeckEditor::actLoadDeck()
{
    auto deckOpenLocation = confirmOpen();

    if (deckOpenLocation == CANCELLED) {
        return;
    }

    DlgLoadDeck dialog(this);
    if (!dialog.exec())
        return;

    QString fileName = dialog.selectedFiles().at(0);
    TabDeckEditor::openDeckFromFile(fileName, deckOpenLocation);
    updateBannerCardComboBox();
}

/**
 * Actually opens the deck from file
 * @param fileName The path of the deck to open
 * @param deckOpenLocation Which tab to open the deck
 */
void TabDeckEditor::openDeckFromFile(const QString &fileName, DeckOpenLocation deckOpenLocation)
{
    DeckLoader::FileFormat fmt = DeckLoader::getFormatFromName(fileName);

    auto *l = new DeckLoader;
    if (l->loadFromFile(fileName, fmt, true)) {
        SettingsCache::instance().recents().updateRecentlyOpenedDeckPaths(fileName);
        updateBannerCardComboBox();
        if (!l->getBannerCard().first.isEmpty()) {
            bannerCardComboBox->setCurrentIndex(bannerCardComboBox->findText(l->getBannerCard().first));
        }
        if (deckOpenLocation == NEW_TAB) {
            emit openDeckEditor(l);
        } else {
            setSaveStatus(false);
            setDeck(l);
        }
    } else {
        delete l;
        QMessageBox::critical(this, tr("Error"), tr("Could not open deck at %1").arg(fileName));
    }
    setSaveStatus(true);
}

// Method uses to sync docks state with menu items state
bool TabDeckEditor::eventFilter(QObject *o, QEvent *e)
{
    if (e->type() == QEvent::Close) {
        if (o == cardInfoDock) {
            aCardInfoDockVisible->setChecked(false);
            aCardInfoDockFloating->setEnabled(false);
        } else if (o == deckDock) {
            aDeckDockVisible->setChecked(false);
            aDeckDockFloating->setEnabled(false);
        } else if (o == filterDock) {
            aFilterDockVisible->setChecked(false);
            aFilterDockFloating->setEnabled(false);
        } else if (o == printingSelectorDock) {
            aPrintingSelectorDockVisible->setChecked(false);
            aPrintingSelectorDockFloating->setEnabled(false);
        }
    }
    if (o == this && e->type() == QEvent::Hide) {
        LayoutsSettings &layouts = SettingsCache::instance().layouts();
        layouts.setDeckEditorLayoutState(saveState());
        layouts.setDeckEditorGeometry(saveGeometry());
        layouts.setDeckEditorCardSize(cardInfoDock->size());
        layouts.setDeckEditorFilterSize(filterDock->size());
        layouts.setDeckEditorDeckSize(deckDock->size());
        layouts.setDeckEditorPrintingSelectorSize(printingSelectorDock->size());
    }
    return false;
}

void TabDeckEditor::dockVisibleTriggered()
{
    QObject *o = sender();
    if (o == aCardInfoDockVisible) {
        cardInfoDock->setHidden(!aCardInfoDockVisible->isChecked());
        aCardInfoDockFloating->setEnabled(aCardInfoDockVisible->isChecked());
        return;
    }

    if (o == aDeckDockVisible) {
        deckDock->setHidden(!aDeckDockVisible->isChecked());
        aDeckDockFloating->setEnabled(aDeckDockVisible->isChecked());
        return;
    }

    if (o == aFilterDockVisible) {
        filterDock->setHidden(!aFilterDockVisible->isChecked());
        aFilterDockFloating->setEnabled(aFilterDockVisible->isChecked());
        return;
    }

    if (o == aPrintingSelectorDockVisible) {
        printingSelectorDock->setHidden(!aPrintingSelectorDockVisible->isChecked());
        aPrintingSelectorDockFloating->setEnabled(aPrintingSelectorDockVisible->isChecked());
        return;
    }
}

void TabDeckEditor::dockFloatingTriggered()
{
    QObject *o = sender();
    if (o == aCardInfoDockFloating) {
        cardInfoDock->setFloating(aCardInfoDockFloating->isChecked());
        return;
    }

    if (o == aDeckDockFloating) {
        deckDock->setFloating(aDeckDockFloating->isChecked());
        return;
    }

    if (o == aFilterDockFloating) {
        filterDock->setFloating(aFilterDockFloating->isChecked());
        return;
    }

    if (o == aPrintingSelectorDockFloating) {
        printingSelectorDock->setFloating(aPrintingSelectorDockFloating->isChecked());
        return;
    }
}

void TabDeckEditor::dockTopLevelChanged(bool topLevel)
{
    QObject *o = sender();
    if (o == cardInfoDock) {
        aCardInfoDockFloating->setChecked(topLevel);
        return;
    }

    if (o == deckDock) {
        aDeckDockFloating->setChecked(topLevel);
        return;
    }

    if (o == filterDock) {
        aFilterDockFloating->setChecked(topLevel);
        return;
    }

    if (o == printingSelectorDock) {
        aPrintingSelectorDockFloating->setChecked(topLevel);
        return;
    }
}