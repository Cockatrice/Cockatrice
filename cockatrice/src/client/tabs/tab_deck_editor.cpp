#include "tab_deck_editor.h"

#include "../../client/game_logic/abstract_client.h"
#include "../../client/tapped_out_interface.h"
#include "../../client/ui/widgets/cards/card_info_frame_widget.h"
#include "../../deck/deck_list_model.h"
#include "../../deck/deck_stats_interface.h"
#include "../../dialogs/dlg_load_deck_from_clipboard.h"
#include "../../game/cards/card_database_manager.h"
#include "../../game/cards/card_database_model.h"
#include "../../game/filters/filter_builder.h"
#include "../../game/filters/filter_tree_model.h"
#include "../../main.h"
#include "../../server/pending_command.h"
#include "../../settings/cache_settings.h"
#include "../ui/picture_loader.h"
#include "../ui/pixel_map_generator.h"
#include "../ui/widgets/printing_selector/printing_selector.h"
#include "pb/command_deck_upload.pb.h"
#include "pb/response.pb.h"
#include "tab_supervisor.h"
#include "trice_limits.h"

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QCloseEvent>
#include <QComboBox>
#include <QDesktopServices>
#include <QDir>
#include <QDockWidget>
#include <QFileDialog>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPrintPreviewDialog>
#include <QProcessEnvironment>
#include <QPushButton>
#include <QRegularExpression>
#include <QSplitter>
#include <QTextBrowser>
#include <QTextEdit>
#include <QTextStream>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>
#include <QTreeView>
#include <QUrl>
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

    auto *upperLayout = new QGridLayout;
    upperLayout->setObjectName("upperLayout");
    upperLayout->addWidget(nameLabel, 0, 0);
    upperLayout->addWidget(nameEdit, 0, 1);

    upperLayout->addWidget(commentsLabel, 1, 0);
    upperLayout->addWidget(commentsEdit, 1, 1);

    upperLayout->addWidget(bannerCardLabel, 2, 0);
    upperLayout->addWidget(bannerCardComboBox, 2, 1);

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
    lowerLayout->addWidget(deckView, 1, 0, 1, 5);

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

    aSaveDeckToClipboardRaw = new QAction(QString(), this);
    connect(aSaveDeckToClipboardRaw, SIGNAL(triggered()), this, SLOT(actSaveDeckToClipboardRaw()));

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
    connect(aClose, SIGNAL(triggered()), this, SLOT(closeRequest()));

    aClearFilterAll = new QAction(QString(), this);
    aClearFilterAll->setIcon(QPixmap("theme:icons/clearsearch"));
    connect(aClearFilterAll, SIGNAL(triggered()), this, SLOT(actClearFilterAll()));

    aClearFilterOne = new QAction(QString(), this);
    aClearFilterOne->setIcon(QPixmap("theme:icons/decrement"));
    connect(aClearFilterOne, SIGNAL(triggered()), this, SLOT(actClearFilterOne()));

    saveDeckToClipboardMenu = new QMenu(this);
    saveDeckToClipboardMenu->addAction(aSaveDeckToClipboard);
    saveDeckToClipboardMenu->addAction(aSaveDeckToClipboardRaw);

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

void TabDeckEditor::databaseCustomMenu(QPoint point)
{
    QMenu menu;
    const CardInfoPtr info = currentCardInfo();

    // add to deck and sideboard options
    QAction *addToDeck, *addToSideboard, *selectPrinting;
    addToDeck = menu.addAction(tr("Add to Deck"));
    addToSideboard = menu.addAction(tr("Add to Sideboard"));
    selectPrinting = menu.addAction(tr("Select Printing"));

    connect(addToDeck, SIGNAL(triggered()), this, SLOT(actAddCard()));
    connect(addToSideboard, SIGNAL(triggered()), this, SLOT(actAddCardToSideboard()));
    connect(selectPrinting, &QAction::triggered, this, [this, info] { this->showPrintingSelector(); });

    // filling out the related cards submenu
    auto *relatedMenu = new QMenu(tr("Show Related cards"));
    menu.addMenu(relatedMenu);
    auto relatedCards = info->getAllRelatedCards();
    if (relatedCards.isEmpty()) {
        relatedMenu->setDisabled(true);
    } else {
        for (const CardRelation *rel : relatedCards) {
            const QString &relatedCardName = rel->getName();
            QAction *relatedCard = relatedMenu->addAction(relatedCardName);
            connect(relatedCard, &QAction::triggered, cardInfo,
                    [this, relatedCardName] { cardInfo->setCard(relatedCardName); });
        }
    }
    menu.exec(databaseView->mapToGlobal(point));
}

void TabDeckEditor::decklistCustomMenu(QPoint point)
{
    QMenu menu;
    const CardInfoPtr info = cardInfo->getInfo();

    QAction *selectPrinting = menu.addAction(tr("Select Printing"));

    connect(selectPrinting, &QAction::triggered, this, &TabDeckEditor::showPrintingSelector);

    menu.exec(deckView->mapToGlobal(point));
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

    aCardInfoDockVisible->setChecked(cardInfoDock->isVisible());
    aFilterDockVisible->setChecked(filterDock->isVisible());
    aDeckDockVisible->setChecked(deckDock->isVisible());
    aPrintingSelectorDockVisible->setChecked(printingSelectorDock->isVisible());

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

TabDeckEditor::TabDeckEditor(TabSupervisor *_tabSupervisor, QWidget *parent)
    : Tab(_tabSupervisor, parent), modified(false)
{
    setObjectName("TabDeckEditor");

    createMenus();

    createCentralFrame();

    createDeckDock();
    createCardInfoDock();
    createFiltersDock();
    createPrintingSelectorDock();

    this->installEventFilter(this);

    retranslateUi();
    connect(&SettingsCache::instance().shortcuts(), SIGNAL(shortCutChanged()), this, SLOT(refreshShortcuts()));
    refreshShortcuts();

    loadLayout();
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

    aNewDeck->setText(tr("&New deck"));
    aLoadDeck->setText(tr("&Load deck..."));
    loadRecentDeckMenu->setTitle(tr("Load recent deck..."));
    aClearRecents->setText(tr("Clear"));
    aSaveDeck->setText(tr("&Save deck"));
    aSaveDeckAs->setText(tr("Save deck &as..."));
    aLoadDeckFromClipboard->setText(tr("Load deck from cl&ipboard..."));

    saveDeckToClipboardMenu->setTitle(tr("Save deck to clipboard"));
    aSaveDeckToClipboard->setText(tr("Annotated"));
    aSaveDeckToClipboardRaw->setText(tr("Not Annotated"));

    aPrintDeck->setText(tr("&Print deck..."));

    analyzeDeckMenu->setTitle(tr("&Send deck to online service"));
    aExportDeckDecklist->setText(tr("Create decklist (decklist.org)"));
    aAnalyzeDeckDeckstats->setText(tr("Analyze deck (deckstats.net)"));
    aAnalyzeDeckTappedout->setText(tr("Analyze deck (tappedout.net)"));

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

void TabDeckEditor::updateName(const QString &name)
{
    deckModel->getDeckList()->setName(name);
    setModified(true);
    setSaveStatus(true);
}

void TabDeckEditor::updateComments()
{
    deckModel->getDeckList()->setComments(commentsEdit->toPlainText());
    setModified(true);
    setSaveStatus(true);
}

void TabDeckEditor::updateBannerCardComboBox()
{
    // Store the current text of the combo box
    QString currentText = bannerCardComboBox->currentText();

    // Block signals temporarily
    bool wasBlocked = bannerCardComboBox->blockSignals(true);

    // Clear the existing items in the combo box
    bannerCardComboBox->clear();

    // Prepare the new items with deduplication
    QSet<QString> bannerCardSet;
    InnerDecklistNode *listRoot = deckModel->getDeckList()->getRoot();
    for (int i = 0; i < listRoot->size(); i++) {
        InnerDecklistNode *currentZone = dynamic_cast<InnerDecklistNode *>(listRoot->at(i));
        for (int j = 0; j < currentZone->size(); j++) {
            DecklistCardNode *currentCard = dynamic_cast<DecklistCardNode *>(currentZone->at(j));
            if (!currentCard)
                continue;

            for (int k = 0; k < currentCard->getNumber(); ++k) {
                CardInfoPtr info = CardDatabaseManager::getInstance()->getCard(currentCard->getName());
                if (info) {
                    bannerCardSet.insert(currentCard->getName());
                }
            }
        }
    }

    // Convert the QSet to a sorted QStringList
    QStringList bannerCardChoices = QStringList(bannerCardSet.begin(), bannerCardSet.end());
    bannerCardChoices.sort(Qt::CaseInsensitive);

    // Populate the combo box with new items
    bannerCardComboBox->addItems(bannerCardChoices);

    // Try to restore the previous selection by finding the currentText
    int restoredIndex = bannerCardComboBox->findText(currentText);
    if (restoredIndex != -1) {
        bannerCardComboBox->setCurrentIndex(restoredIndex);
    } else {
        // Add a placeholder "-" and set it as the current selection
        int bannerIndex = bannerCardComboBox->findText(deckModel->getDeckList()->getBannerCard());
        if (bannerIndex != -1) {
            bannerCardComboBox->setCurrentIndex(bannerIndex);
        } else {
            bannerCardComboBox->insertItem(0, "-");
            bannerCardComboBox->setCurrentIndex(0);
        }
    }

    // Restore the previous signal blocking state
    bannerCardComboBox->blockSignals(wasBlocked);
}

void TabDeckEditor::setBannerCard()
{
    qDebug() << "Banner card was set to: " << bannerCardComboBox->currentText();
    deckModel->getDeckList()->setBannerCard(bannerCardComboBox->currentText());
}

void TabDeckEditor::updateCardInfo(CardInfoPtr _card)
{
    cardInfo->setCard(_card);
}

void TabDeckEditor::updateCardInfoLeft(const QModelIndex &current, const QModelIndex & /*previous*/)
{
    cardInfo->setCard(current.sibling(current.row(), 0).data().toString());
}

void TabDeckEditor::updateCardInfoRight(const QModelIndex &current, const QModelIndex & /*previous*/)
{
    if (!current.isValid())
        return;
    if (!current.model()->hasChildren(current.sibling(current.row(), 0))) {
        cardInfo->setCard(current.sibling(current.row(), 1).data().toString(),
                          current.sibling(current.row(), 4).data().toString());
    }
}

void TabDeckEditor::updatePrintingSelectorDatabase(const QModelIndex &current, const QModelIndex & /*previous*/)
{
    const QString cardName = current.sibling(current.row(), 0).data().toString();
    const QString cardProviderID = CardDatabaseManager::getInstance()->getPreferredPrintingProviderIdForCard(cardName);

    if (!current.isValid()) {
        return;
    }

    if (!current.model()->hasChildren(current.sibling(current.row(), 0))) {
        printingSelector->setCard(
            CardDatabaseManager::getInstance()->getCardByNameAndProviderId(cardName, cardProviderID), DECK_ZONE_MAIN);
    }
}

void TabDeckEditor::updatePrintingSelectorDeckView(const QModelIndex &current, const QModelIndex & /*previous*/)
{
    const QString cardName = current.sibling(current.row(), 1).data().toString();
    const QString cardProviderID = current.sibling(current.row(), 4).data().toString();
    const QModelIndex gparent = current.parent().parent();

    if (!gparent.isValid()) {
        return;
    }

    const QString zoneName = gparent.sibling(gparent.row(), 1).data(Qt::EditRole).toString();

    if (!current.isValid()) {
        return;
    }

    if (!current.model()->hasChildren(current.sibling(current.row(), 0))) {
        printingSelector->setCard(
            CardDatabaseManager::getInstance()->getCardByNameAndProviderId(cardName, cardProviderID), zoneName);
    }
}

void TabDeckEditor::updateSearch(const QString &search)
{
    databaseDisplayModel->setStringFilter(search);
    QModelIndexList sel = databaseView->selectionModel()->selectedRows();
    if (sel.isEmpty() && databaseDisplayModel->rowCount())
        databaseView->selectionModel()->setCurrentIndex(databaseDisplayModel->index(0, 0),
                                                        QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
}

void TabDeckEditor::updateHash()
{
    hashLabel->setText(deckModel->getDeckList()->getDeckHash());
}

void TabDeckEditor::updateRecentlyOpened()
{
    loadRecentDeckMenu->clear();
    for (const auto &deckPath : SettingsCache::instance().recents().getRecentlyOpenedDeckPaths()) {
        QAction *aRecentlyOpenedDeck = new QAction(deckPath, this);
        loadRecentDeckMenu->addAction(aRecentlyOpenedDeck);
        connect(aRecentlyOpenedDeck, &QAction::triggered, this,
                [=, this] { actOpenRecent(aRecentlyOpenedDeck->text()); });
    }
    loadRecentDeckMenu->addSeparator();
    loadRecentDeckMenu->addAction(aClearRecents);
    aClearRecents->setEnabled(SettingsCache::instance().recents().getRecentlyOpenedDeckPaths().length() > 0);
}

bool TabDeckEditor::confirmClose()
{
    if (modified) {
        tabSupervisor->setCurrentWidget(this);
        int ret = createSaveConfirmationWindow()->exec();
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

    QFileDialog dialog(this, tr("Load deck"));
    dialog.setDirectory(SettingsCache::instance().getDeckPath());
    dialog.setNameFilters(DeckLoader::fileNameFilters);
    if (!dialog.exec())
        return;

    QString fileName = dialog.selectedFiles().at(0);
    openDeckFromFile(fileName, deckOpenLocation);
}

void TabDeckEditor::actOpenRecent(const QString &fileName)
{
    auto deckOpenLocation = confirmOpen();

    if (deckOpenLocation == CANCELLED) {
        return;
    }

    openDeckFromFile(fileName, deckOpenLocation);
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
    if (l->loadFromFile(fileName, fmt)) {
        SettingsCache::instance().recents().updateRecentlyOpenedDeckPaths(fileName);
        if (deckOpenLocation == NEW_TAB) {
            emit openDeckEditor(l);
        } else {
            setSaveStatus(false);
            setDeck(l);
        }
    } else {
        delete l;
    updateBannerCardComboBox();
    if (!l->getBannerCard().isEmpty()) {
        qDebug() << "Found banner card:" << l->getBannerCard();
        bannerCardComboBox->setCurrentIndex(bannerCardComboBox->findText(l->getBannerCard()));
    }
        QMessageBox::critical(this, tr("Error"), tr("Could not open deck at %1").arg(fileName));
    }
    setSaveStatus(true);
}

void TabDeckEditor::actClearRecents()
{
    SettingsCache::instance().recents().clearRecentlyOpenedDeckPaths();
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
        QString deckString = deck->writeToString_Native();
        if (deckString.length() > MAX_FILE_LENGTH) {
            QMessageBox::critical(this, tr("Error"), tr("Could not save remote deck"));
            return false;
        }

        Command_DeckUpload cmd;
        cmd.set_deck_id(static_cast<google::protobuf::uint32>(deck->getLastRemoteDeckId()));
        cmd.set_deck_list(deckString.toStdString());

        PendingCommand *pend = AbstractClient::prepareSessionCommand(cmd);
        connect(pend, SIGNAL(finished(Response, CommandContainer, QVariant)), this,
                SLOT(saveDeckRemoteFinished(Response)));
        tabSupervisor->getClient()->sendCommand(pend);

        return true;
    } else if (deck->getLastFileName().isEmpty())
        return actSaveDeckAs();
    else if (deck->saveToFile(deck->getLastFileName(), deck->getLastFileFormat())) {
        setModified(false);
        return true;
    }
    QMessageBox::critical(
        this, tr("Error"),
        tr("The deck could not be saved.\nPlease check that the directory is writable and try again."));
    return false;
}

bool TabDeckEditor::actSaveDeckAs()
{
    QFileDialog dialog(this, tr("Save deck"));
    dialog.setDirectory(SettingsCache::instance().getDeckPath());
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setDefaultSuffix("cod");
    dialog.setNameFilters(DeckLoader::fileNameFilters);
    dialog.selectFile(deckModel->getDeckList()->getName().trimmed() + ".cod");
    if (!dialog.exec())
        return false;

    QString fileName = dialog.selectedFiles().at(0);
    DeckLoader::FileFormat fmt = DeckLoader::getFormatFromName(fileName);

    if (!deckModel->getDeckList()->saveToFile(fileName, fmt)) {
        QMessageBox::critical(
            this, tr("Error"),
            tr("The deck could not be saved.\nPlease check that the directory is writable and try again."));
        return false;
    }
    setModified(false);

    SettingsCache::instance().recents().updateRecentlyOpenedDeckPaths(fileName);

    return true;
}

void TabDeckEditor::actLoadDeckFromClipboard()
{
    auto deckOpenLocation = confirmOpen();

    if (deckOpenLocation == CANCELLED) {
        return;
    }

    DlgLoadDeckFromClipboard dlg(this);
    if (!dlg.exec())
        return;

    if (deckOpenLocation == NEW_TAB) {
        emit openDeckEditor(dlg.getDeckList());
    } else {
        setDeck(dlg.getDeckList());
        setModified(true);
    }

    setSaveStatus(true);
}

void TabDeckEditor::actSaveDeckToClipboard()
{
    QString buffer;
    QTextStream stream(&buffer);
    deckModel->getDeckList()->saveToStream_Plain(stream);
    QApplication::clipboard()->setText(buffer, QClipboard::Clipboard);
    QApplication::clipboard()->setText(buffer, QClipboard::Selection);
}

void TabDeckEditor::actSaveDeckToClipboardRaw()
{
    QString buffer;
    QTextStream stream(&buffer);
    deckModel->getDeckList()->saveToStream_Plain(stream, false);
    QApplication::clipboard()->setText(buffer, QClipboard::Clipboard);
    QApplication::clipboard()->setText(buffer, QClipboard::Selection);
}

void TabDeckEditor::actPrintDeck()
{
    auto *dlg = new QPrintPreviewDialog(this);
    connect(dlg, SIGNAL(paintRequested(QPrinter *)), deckModel, SLOT(printDeckList(QPrinter *)));
    dlg->exec();
}

// Action called when export deck to decklist menu item is pressed.
void TabDeckEditor::actExportDeckDecklist()
{
    // Get the decklist class for the deck.
    DeckLoader *const deck = deckModel->getDeckList();
    // create a string to load the decklist url into.
    QString decklistUrlString;
    // check if deck is not null
    if (deck) {
        // Get the decklist url string from the deck loader class.
        decklistUrlString = deck->exportDeckToDecklist();
        // Check to make sure the string isn't empty.
        if (QString::compare(decklistUrlString, "", Qt::CaseInsensitive) == 0) {
            // Show an error if the deck is empty, and return.
            QMessageBox::critical(this, tr("Error"), tr("There are no cards in your deck to be exported"));
            return;
        }
        // Encode the string recieved from the model to make sure all characters are encoded.
        // first we put it into a qurl object
        QUrl decklistUrl = QUrl(decklistUrlString);
        // we get the correctly encoded url.
        decklistUrlString = decklistUrl.toEncoded();
        // We open the url in the user's default browser
        QDesktopServices::openUrl(decklistUrlString);
    } else {
        // if there's no deck loader object, return an error
        QMessageBox::critical(this, tr("Error"), tr("No deck was selected to be saved."));
    }
}

void TabDeckEditor::actAnalyzeDeckDeckstats()
{
    auto *interface = new DeckStatsInterface(*databaseModel->getDatabase(),
                                             this); // it deletes itself when done
    interface->analyzeDeck(deckModel->getDeckList());
}

void TabDeckEditor::actAnalyzeDeckTappedout()
{
    auto *interface = new TappedOutInterface(*databaseModel->getDatabase(),
                                             this); // it deletes itself when done
    interface->analyzeDeck(deckModel->getDeckList());
}

void TabDeckEditor::actClearFilterAll()
{
    databaseDisplayModel->clearFilterAll();
    searchEdit->setText("");
}

void TabDeckEditor::actClearFilterOne()
{
    QModelIndexList selIndexes = filterView->selectionModel()->selectedIndexes();
    for (QModelIndex idx : selIndexes) {
        filterModel->removeRow(idx.row(), idx.parent());
    }
}

void TabDeckEditor::recursiveExpand(const QModelIndex &index)
{
    if (index.parent().isValid())
        recursiveExpand(index.parent());
    deckView->expand(index);
}

/**
 * @brief Displays the save confirmation dialogue that is shown before loading a deck, if required. Takes into
 * account the `openDeckInNewTab` settting.
 *
 * @param openInSameTabIfBlank Open the deck in the same tab instead of a new tab if the current tab is completely
 * blank. Only relevant when the `openDeckInNewTab` setting is enabled.
 *
 * @returns An enum that indicates if and where to load the deck
 */
TabDeckEditor::DeckOpenLocation TabDeckEditor::confirmOpen(const bool openInSameTabIfBlank)
{
    // handle `openDeckInNewTab` setting
    if (SettingsCache::instance().getOpenDeckInNewTab()) {
        if (openInSameTabIfBlank && isBlankNewDeck()) {
            return SAME_TAB;
        } else {
            return NEW_TAB;
        }
    }

    // early return if deck is unmodified
    if (!modified) {
        return SAME_TAB;
    }

    // do the save confirmation dialogue
    tabSupervisor->setCurrentWidget(this);

    QMessageBox *msgBox = createSaveConfirmationWindow();
    QPushButton *newTabButton = msgBox->addButton(tr("Open in new tab"), QMessageBox::ApplyRole);

    int ret = msgBox->exec();

    // `exec()` returns an opaque value if a non-standard button was clicked.
    // Directly check if newTabButton was clicked before switching over the standard buttons.
    if (msgBox->clickedButton() == newTabButton) {
        return NEW_TAB;
    }

    switch (ret) {
        case QMessageBox::Save:
            return actSaveDeck() ? SAME_TAB : CANCELLED;
        case QMessageBox::Discard:
            return SAME_TAB;
        default:
            return CANCELLED;
    }
}

/**
 * @brief Creates the base save confirmation dialogue box.
 *
 * @returns A QMessageBox that can be further modified
 */
QMessageBox *TabDeckEditor::createSaveConfirmationWindow()
{
    QMessageBox *msgBox = new QMessageBox(this);
    msgBox->setIcon(QMessageBox::Warning);
    msgBox->setWindowTitle(tr("Are you sure?"));
    msgBox->setText(tr("The decklist has been modified.\nDo you want to save the changes?"));
    msgBox->setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    return msgBox;
}

/**
 * @brief Returns true if this tab is a blank newly opened tab, as if it was just created with the `New Deck` action.
 */
bool TabDeckEditor::isBlankNewDeck() const
{
    DeckLoader *const deck = deckModel->getDeckList();
    return !modified && deck->getLastFileName().isEmpty() && deck->getLastRemoteDeckId() == -1;
}

CardInfoPtr TabDeckEditor::currentCardInfo() const
{
    const QModelIndex currentIndex = databaseView->selectionModel()->currentIndex();
    if (!currentIndex.isValid()) {
        return {};
    }

    const QString cardName = currentIndex.sibling(currentIndex.row(), 0).data().toString();

    return CardDatabaseManager::getInstance()->getCard(cardName);
}

/**
 * Gets the index of all the currently selected card nodes in the decklist table.
 * The list is in reverse order of the visual selection, so that rows can be deleted while iterating over them.
 *
 * @return A model index list containing all selected card nodes
 */
QModelIndexList TabDeckEditor::getSelectedCardNodes() const
{
    auto selectedRows = deckView->selectionModel()->selectedRows();

    const auto notLeafNode = [this](const auto &index) { return deckModel->hasChildren(index); };
    selectedRows.erase(std::remove_if(selectedRows.begin(), selectedRows.end(), notLeafNode), selectedRows.end());

    std::reverse(selectedRows.begin(), selectedRows.end());
    return selectedRows;
}

void TabDeckEditor::addCardHelper(QString zoneName)
{
    const CardInfoPtr info = currentCardInfo();
    if (!info)
        return;
    if (info->getIsToken())
        zoneName = DECK_ZONE_TOKENS;

    QModelIndex newCardIndex = deckModel->addPreferredPrintingCard(info->getName(), zoneName, false);
    recursiveExpand(newCardIndex);
    deckView->clearSelection();
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
    const QString cardProviderID = currentIndex.sibling(currentIndex.row(), 4).data().toString();
    const QModelIndex gparent = currentIndex.parent().parent();

    if (!gparent.isValid())
        return;

    const QString zoneName = gparent.sibling(gparent.row(), 1).data(Qt::EditRole).toString();
    actDecrement();
    const QString otherZoneName = zoneName == DECK_ZONE_MAIN ? DECK_ZONE_SIDE : DECK_ZONE_MAIN;

    // Third argument (true) says create the card no matter what, even if not in DB
    QModelIndex newCardIndex = deckModel->addCard(
        cardName, CardDatabaseManager::getInstance()->getSpecificSetForCard(cardName, cardProviderID), otherZoneName,
        true);
    recursiveExpand(newCardIndex);

    setModified(true);
    setSaveStatus(true);
}

void TabDeckEditor::actAddCard()
{
    if (QApplication::keyboardModifiers() & Qt::ControlModifier)
        actAddCardToSideboard();
    else
        addCardHelper(DECK_ZONE_MAIN);
    setSaveStatus(true);
}

void TabDeckEditor::actAddCardToSideboard()
{
    addCardHelper(DECK_ZONE_SIDE);
    setSaveStatus(true);
}

void TabDeckEditor::actRemoveCard()
{
    auto selectedRows = getSelectedCardNodes();

    // hack to maintain the old reselection behavior when currently selected row of a single-selection gets deleted
    // TODO: remove the hack and also handle reselection when all rows of a multi-selection gets deleted
    if (selectedRows.length() == 1) {
        deckView->setSelectionMode(QAbstractItemView::SingleSelection);
    }

    bool modified = false;
    for (const auto &index : selectedRows) {
        if (!index.isValid() || deckModel->hasChildren(index)) {
            continue;
        }
        deckModel->removeRow(index.row(), index.parent());
        modified = true;
    }

    deckView->setSelectionMode(QAbstractItemView::ExtendedSelection);

    if (modified) {
        DeckLoader *const deck = deckModel->getDeckList();
        setSaveStatus(!deck->isEmpty());
        setModified(true);
    }
}

void TabDeckEditor::offsetCountAtIndex(const QModelIndex &idx, int offset)
{
    if (!idx.isValid() || deckModel->hasChildren(idx)) {
        return;
    }

    const QModelIndex numberIndex = idx.sibling(idx.row(), 0);
    const int count = deckModel->data(numberIndex, Qt::EditRole).toInt();
    const int new_count = count + offset;
    if (new_count <= 0)
        deckModel->removeRow(idx.row(), idx.parent());
    else
        deckModel->setData(numberIndex, new_count, Qt::EditRole);
    setModified(true);
}

void TabDeckEditor::decrementCardHelper(QString zoneName)
{
    const CardInfoPtr info = currentCardInfo();

    if (!info)
        return;
    if (info->getIsToken())
        zoneName = DECK_ZONE_TOKENS;

    QModelIndex idx = deckModel->findCard(info->getName(), zoneName);
    if (!idx.isValid()) {
        return;
    }
    deckView->clearSelection();
    deckView->setCurrentIndex(idx);
    offsetCountAtIndex(idx, -1);
}

void TabDeckEditor::actDecrementCard()
{
    decrementCardHelper(DECK_ZONE_MAIN);
}

void TabDeckEditor::actDecrementCardFromSideboard()
{
    decrementCardHelper(DECK_ZONE_SIDE);
}

void TabDeckEditor::copyDatabaseCellContents()
{
    auto _data = databaseView->selectionModel()->currentIndex().data();
    QApplication::clipboard()->setText(_data.toString());
}

void TabDeckEditor::actIncrement()
{
    auto selectedRows = getSelectedCardNodes();

    for (const auto &index : selectedRows) {
        offsetCountAtIndex(index, 1);
    }
}

void TabDeckEditor::actDecrement()
{
    auto selectedRows = getSelectedCardNodes();

    // hack to maintain the old reselection behavior when currently selected row of a single-selection gets deleted
    // TODO: remove the hack and also handle reselection when all rows of a multi-selection gets deleted
    if (selectedRows.length() == 1) {
        deckView->setSelectionMode(QAbstractItemView::SingleSelection);
    }

    for (const auto &index : selectedRows) {
        offsetCountAtIndex(index, -1);
    }

    deckView->setSelectionMode(QAbstractItemView::ExtendedSelection);
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

    PictureLoader::cacheCardPixmaps(
        CardDatabaseManager::getInstance()->getCards(deckModel->getDeckList()->getCardList()));
    deckView->expandAll();
    setModified(false);

    // If they load a deck, make the deck list appear
    aDeckDockVisible->setChecked(true);
    deckDock->setVisible(aDeckDockVisible->isChecked());
}

void TabDeckEditor::setModified(bool _modified)
{
    modified = _modified;
    emit tabTextChanged(this, getTabText());
}

void TabDeckEditor::filterViewCustomContextMenu(const QPoint &point)
{
    QMenu menu;
    QAction *action;
    QModelIndex idx;

    idx = filterView->indexAt(point);
    if (!idx.isValid())
        return;

    action = menu.addAction(QString("delete"));
    action->setData(point);
    connect(&menu, SIGNAL(triggered(QAction *)), this, SLOT(filterRemove(QAction *)));
    menu.exec(filterView->mapToGlobal(point));
}

void TabDeckEditor::filterRemove(QAction *action)
{
    QPoint point;
    QModelIndex idx;

    point = action->data().toPoint();
    idx = filterView->indexAt(point);
    if (!idx.isValid())
        return;

    filterModel->removeRow(idx.row(), idx.parent());
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
        cardInfoDock->setVisible(aCardInfoDockVisible->isChecked());
        aCardInfoDockFloating->setEnabled(aCardInfoDockVisible->isChecked());
        return;
    }

    if (o == aDeckDockVisible) {
        deckDock->setVisible(aDeckDockVisible->isChecked());
        aDeckDockFloating->setEnabled(aDeckDockVisible->isChecked());
        return;
    }

    if (o == aFilterDockVisible) {
        filterDock->setVisible(aFilterDockVisible->isChecked());
        aFilterDockFloating->setEnabled(aFilterDockVisible->isChecked());
        return;
    }

    if (o == aPrintingSelectorDockVisible) {
        printingSelectorDock->setVisible(aPrintingSelectorDockVisible->isChecked());
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

void TabDeckEditor::saveDbHeaderState()
{
    SettingsCache::instance().layouts().setDeckEditorDbHeaderState(databaseView->header()->saveState());
}

void TabDeckEditor::setSaveStatus(bool newStatus)
{
    aSaveDeck->setEnabled(newStatus);
    aSaveDeckAs->setEnabled(newStatus);
    aSaveDeckToClipboard->setEnabled(newStatus);
    aSaveDeckToClipboardRaw->setEnabled(newStatus);
    saveDeckToClipboardMenu->setEnabled(newStatus);
    aPrintDeck->setEnabled(newStatus);
    analyzeDeckMenu->setEnabled(newStatus);
}

void TabDeckEditor::showSearchSyntaxHelp()
{

    QFile file("theme:help/search.md");

    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        return;
    }

    QTextStream in(&file);
    QString text = in.readAll();
    file.close();

    // Poor Markdown Converter
    auto opts = QRegularExpression::MultilineOption;
    text = text.replace(QRegularExpression("^(###)(.*)", opts), "<h3>\\2</h3>")
               .replace(QRegularExpression("^(##)(.*)", opts), "<h2>\\2</h2>")
               .replace(QRegularExpression("^(#)(.*)", opts), "<h1>\\2</h1>")
               .replace(QRegularExpression("^------*", opts), "<hr />")
               .replace(QRegularExpression(R"(\[([^[]+)\]\(([^\)]+)\))", opts), R"(<a href='\2'>\1</a>)");

    auto browser = new QTextBrowser;
    browser->setParent(this, Qt::Window | Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowMinMaxButtonsHint |
                                 Qt::WindowCloseButtonHint | Qt::WindowFullscreenButtonHint);
    browser->setWindowTitle("Search Help");
    browser->setReadOnly(true);
    browser->setMinimumSize({500, 600});

    QString sheet = QString("a { text-decoration: underline; color: rgb(71,158,252) };");
    browser->document()->setDefaultStyleSheet(sheet);

    browser->setHtml(text);
    connect(browser, &QTextBrowser::anchorClicked, [this](const QUrl &link) { searchEdit->setText(link.fragment()); });
    browser->show();
}
