#include "tab_deck_editor_visual.h"

#include "../../../deck/deck_list_model.h"
#include "../../../deck/deck_stats_interface.h"
#include "../../../dialogs/dlg_load_deck.h"
#include "../../../dialogs/dlg_load_deck_from_clipboard.h"
#include "../../../game/cards/card_database_manager.h"
#include "../../../game/cards/card_database_model.h"
#include "../../../game/filters/filter_builder.h"
#include "../../../game/filters/filter_tree_model.h"
#include "../../../main.h"
#include "../../../server/pending_command.h"
#include "../../../settings/cache_settings.h"
#include "../../game_logic/abstract_client.h"
#include "../../tapped_out_interface.h"
#include "../../ui/picture_loader/picture_loader.h"
#include "../../ui/pixel_map_generator.h"
#include "../../ui/widgets/cards/card_info_frame_widget.h"
#include "../../ui/widgets/deck_analytics/deck_analytics_widget.h"
#include "../../ui/widgets/visual_deck_editor/visual_deck_editor_widget.h"
#include "../tab_deck_editor.h"
#include "../tab_supervisor.h"
#include "pb/command_deck_upload.pb.h"
#include "pb/response.pb.h"
#include "tab_deck_editor_visual_tab_widget.h"
#include "trice_limits.h"

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QCloseEvent>
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

TabDeckEditorVisual::TabDeckEditorVisual(TabSupervisor *_tabSupervisor) : TabGenericDeckEditor(_tabSupervisor)
{
    setObjectName("TabDeckEditorVisual");

    createSearchAndDatabaseFrame();

    deckDockWidget = new DeckEditorDeckDockWidget(this, this);
    cardInfoDockWidget = new DeckEditorCardInfoDockWidget(this);
    filterDockWidget = new DeckEditorFilterDockWidget(this, this);
    printingSelectorDockWidget = new DeckEditorPrintingSelectorDockWidget(this, this);

    TabDeckEditorVisual::createCentralFrame();

    TabDeckEditorVisual::createMenus();

    this->installEventFilter(this);

    TabDeckEditorVisual::retranslateUi();
    connect(&SettingsCache::instance().shortcuts(), SIGNAL(shortCutChanged()), this, SLOT(refreshShortcuts()));
    TabDeckEditorVisual::refreshShortcuts();

    TabDeckEditorVisual::loadLayout();
}

void TabDeckEditorVisual::createCentralFrame()
{
    centralWidget = new QWidget(this);
    centralWidget->setObjectName("centralWidget");

    centralFrame = new QVBoxLayout;
    centralWidget->setLayout(centralFrame);

    tabContainer = new TabDeckEditorVisualTabWidget(centralWidget, this->deckDockWidget->deckModel, this->databaseModel,
                                                    this->databaseDisplayModel);
    connect(tabContainer, SIGNAL(cardChanged(CardInfoPtr)), this, SLOT(changeModelIndexAndCardInfo(CardInfoPtr)));
    connect(tabContainer, SIGNAL(cardChangedDatabaseDisplay(CardInfoPtr)), this, SLOT(setCurrentCardInfo(CardInfoPtr)));
    connect(tabContainer, SIGNAL(mainboardCardClicked(QMouseEvent *, CardInfoPictureWithTextOverlayWidget *)), this,
            SLOT(processMainboardCardClick(QMouseEvent *, CardInfoPictureWithTextOverlayWidget *)));
    connect(tabContainer, SIGNAL(sideboardCardClicked(QMouseEvent *, CardInfoPictureWithTextOverlayWidget *)), this,
            SLOT(processSideboardCardClick(QMouseEvent *, CardInfoPictureWithTextOverlayWidget *)));

    connect(tabContainer, SIGNAL(cardClickedDatabaseDisplay(QMouseEvent *, CardInfoPictureWithTextOverlayWidget *)),
            this, SLOT(processCardClickDatabaseDisplay(QMouseEvent *, CardInfoPictureWithTextOverlayWidget *)));
    centralFrame->addWidget(tabContainer);

    setCentralWidget(centralWidget);
    setDockOptions(QMainWindow::AnimatedDocks | QMainWindow::AllowNestedDocks | QMainWindow::AllowTabbedDocks);
}

void TabDeckEditorVisual::createSearchAndDatabaseFrame()
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
    connect(help, &QAction::triggered, this, &TabDeckEditorVisual::showSearchSyntaxHelp);

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

    searchAndDatabaseFrame = new QVBoxLayout;
    searchAndDatabaseFrame->setObjectName("searchAndDatabaseFrame");
    searchAndDatabaseFrame->addLayout(searchLayout);
    searchAndDatabaseFrame->addWidget(databaseView);

    searchAndDatabaseDock = new QDockWidget(this);
    searchAndDatabaseDock->setObjectName("searchAndDatabaseDock");

    searchAndDatabaseDock->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetFloatable |
                                       QDockWidget::DockWidgetMovable);
    auto *searchAndDatabaseDockContents = new QWidget(this);
    searchAndDatabaseDockContents->setObjectName("filterDockContents");
    searchAndDatabaseDockContents->setLayout(searchAndDatabaseFrame);
    searchAndDatabaseDock->setWidget(searchAndDatabaseDockContents);

    searchAndDatabaseDock->installEventFilter(this);
    connect(searchAndDatabaseDock, SIGNAL(topLevelChanged(bool)), this, SLOT(dockTopLevelChanged(bool)));
}

void TabDeckEditorVisual::createMenus()
{
    deckMenu = new DeckEditorMenu(this, this);
    addTabMenu(deckMenu);

    viewMenu = new QMenu(this);

    cardInfoDockMenu = viewMenu->addMenu(QString());
    deckDockMenu = viewMenu->addMenu(QString());
    deckAnalyticsMenu = viewMenu->addMenu(QString());
    filterDockMenu = viewMenu->addMenu(QString());

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

    aDeckAnalyticsDockVisible = deckAnalyticsMenu->addAction(QString());
    aDeckAnalyticsDockVisible->setCheckable(true);
    connect(aDeckAnalyticsDockVisible, SIGNAL(triggered()), this, SLOT(dockVisibleTriggered()));
    aDeckAnalyticsDockFloating = deckAnalyticsMenu->addAction(QString());
    aDeckAnalyticsDockFloating->setCheckable(true);
    connect(aDeckAnalyticsDockFloating, SIGNAL(triggered()), this, SLOT(dockFloatingTriggered()));

    aFilterDockVisible = filterDockMenu->addAction(QString());
    aFilterDockVisible->setCheckable(true);
    connect(aFilterDockVisible, SIGNAL(triggered()), this, SLOT(dockVisibleTriggered()));
    aFilterDockFloating = filterDockMenu->addAction(QString());
    aFilterDockFloating->setCheckable(true);
    connect(aFilterDockFloating, SIGNAL(triggered()), this, SLOT(dockFloatingTriggered()));

    viewMenu->addSeparator();

    aResetLayout = viewMenu->addAction(QString());
    connect(aResetLayout, SIGNAL(triggered()), this, SLOT(restartLayout()));
    viewMenu->addAction(aResetLayout);

    deckMenu->setSaveStatus(false);

    addTabMenu(viewMenu);
}

QString TabDeckEditorVisual::getTabText() const
{
    QString result = tr("Visual Deck: %1").arg(deckDockWidget->getSimpleDeckName());
    if (modified)
        result.prepend("* ");
    return result;
}

void TabDeckEditorVisual::changeModelIndexAndCardInfo(CardInfoPtr activeCard)
{
    updateCardInfo(activeCard);
    changeModelIndexToCard(activeCard);
}

void TabDeckEditorVisual::changeModelIndexToCard(CardInfoPtr activeCard)
{
    QString cardName = activeCard->getName();
    QModelIndex index = deckDockWidget->deckModel->findCard(cardName, DECK_ZONE_MAIN);
    if (!index.isValid()) {
        index = deckDockWidget->deckModel->findCard(cardName, DECK_ZONE_SIDE);
    }
    deckDockWidget->deckView->setCurrentIndex(index);
}

void TabDeckEditorVisual::processMainboardCardClick(QMouseEvent *event, CardInfoPictureWithTextOverlayWidget *instance)
{
    (void)instance;
    if (event->button() == Qt::LeftButton) {
        deckDockWidget->actSwapCard();
    } else if (event->button() == Qt::RightButton) {
        deckDockWidget->actDecrement();
    } else if (event->button() == Qt::MiddleButton) {
        deckDockWidget->actRemoveCard();
    }
}

void TabDeckEditorVisual::processSideboardCardClick(QMouseEvent *event, CardInfoPictureWithTextOverlayWidget *instance)
{
    (void)instance;
    if (event->button() == Qt::LeftButton) {
        deckDockWidget->actSwapCard();
    } else if (event->button() == Qt::RightButton) {
        deckDockWidget->actDecrement();
    } else if (event->button() == Qt::MiddleButton) {
        deckDockWidget->actRemoveCard();
    }
}

void TabDeckEditorVisual::processCardClickDatabaseDisplay(QMouseEvent *event,
                                                          CardInfoPictureWithTextOverlayWidget *instance)
{
    (void)instance;
    if (event->button() == Qt::LeftButton) {
        // addCardInfo(instance->getInfo(), DECK_ZONE_MAIN);
    } else if (event->button() == Qt::RightButton) {
        deckDockWidget->actDecrement();
    } else if (event->button() == Qt::MiddleButton) {
        deckDockWidget->actRemoveCard();
    }
}

void TabDeckEditorVisual::restartLayout()
{
    deckDockWidget->setVisible(true);
    cardInfoDockWidget->setVisible(true);
    filterDockWidget->setVisible(true);

    deckDockWidget->setFloating(false);
    cardInfoDockWidget->setFloating(false);
    filterDockWidget->setFloating(false);

    aCardInfoDockVisible->setChecked(true);
    aDeckDockVisible->setChecked(true);
    aFilterDockVisible->setChecked(true);

    aCardInfoDockFloating->setChecked(false);
    aDeckDockFloating->setChecked(false);
    aFilterDockFloating->setChecked(false);

    addDockWidget(static_cast<Qt::DockWidgetArea>(2), deckDockWidget);
    addDockWidget(static_cast<Qt::DockWidgetArea>(2), cardInfoDockWidget);
    addDockWidget(static_cast<Qt::DockWidgetArea>(1), deckAnalyticsDock);
    addDockWidget(static_cast<Qt::DockWidgetArea>(2), filterDockWidget);

    splitDockWidget(cardInfoDockWidget, deckDockWidget, Qt::Horizontal);
    splitDockWidget(cardInfoDockWidget, filterDockWidget, Qt::Vertical);
    splitDockWidget(searchAndDatabaseDock, deckAnalyticsDock, Qt::Vertical);

    deckDockWidget->setMinimumWidth(360);
    deckDockWidget->setMaximumWidth(360);

    cardInfoDockWidget->setMinimumSize(250, 360);
    cardInfoDockWidget->setMaximumSize(250, 360);
    QTimer::singleShot(100, this, SLOT(freeDocksSize()));
}

void TabDeckEditorVisual::freeDocksSize()
{
    deckDockWidget->setMinimumSize(100, 100);
    deckDockWidget->setMaximumSize(5000, 5000);

    cardInfoDockWidget->setMinimumSize(100, 100);
    cardInfoDockWidget->setMaximumSize(5000, 5000);

    filterDockWidget->setMinimumSize(100, 100);
    filterDockWidget->setMaximumSize(5000, 5000);
}

void TabDeckEditorVisual::refreshShortcuts()
{
    ShortcutsSettings &shortcuts = SettingsCache::instance().shortcuts();
    aResetLayout->setShortcuts(shortcuts.getShortcut("TabDeckEditorVisual/aResetLayout"));
}

void TabDeckEditorVisual::loadLayout()
{
    LayoutsSettings &layouts = SettingsCache::instance().layouts();
    auto &layoutState = layouts.getDeckEditorLayoutState();
    if (layoutState.isNull()) {
        restartLayout();
    } else {
        restoreState(layoutState);
        restoreGeometry(layouts.getDeckEditorGeometry());
    }

    aCardInfoDockVisible->setChecked(cardInfoDockWidget->isVisible());
    aFilterDockVisible->setChecked(filterDockWidget->isVisible());
    aDeckDockVisible->setChecked(deckDockWidget->isVisible());

    aCardInfoDockFloating->setEnabled(aCardInfoDockVisible->isChecked());
    aDeckDockFloating->setEnabled(aDeckDockVisible->isChecked());
    aFilterDockFloating->setEnabled(aFilterDockVisible->isChecked());

    aCardInfoDockFloating->setChecked(cardInfoDockWidget->isFloating());
    aFilterDockFloating->setChecked(filterDockWidget->isFloating());
    aDeckDockFloating->setChecked(deckDockWidget->isFloating());

    cardInfoDockWidget->setMinimumSize(layouts.getDeckEditorCardSize());
    cardInfoDockWidget->setMaximumSize(layouts.getDeckEditorCardSize());

    filterDockWidget->setMinimumSize(layouts.getDeckEditorFilterSize());
    filterDockWidget->setMaximumSize(layouts.getDeckEditorFilterSize());

    deckDockWidget->setMinimumSize(layouts.getDeckEditorDeckSize());
    deckDockWidget->setMaximumSize(layouts.getDeckEditorDeckSize());

    QTimer::singleShot(100, this, SLOT(freeDocksSize()));
}

void TabDeckEditorVisual::retranslateUi()
{
    aAddCard->setText(tr("Add card to &maindeck"));
    aAddCardToSideboard->setText(tr("Add card to &sideboard"));

    deckMenu->setTitle(tr("&Visual Deck Editor"));

    cardInfoDockWidget->setWindowTitle(tr("Card Info"));
    deckDockWidget->setWindowTitle(tr("Deck"));
    filterDockWidget->setWindowTitle(tr("Filters"));

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

void TabDeckEditorVisual::actNewDeck()
{
    auto deckOpenLocation = confirmOpen(false);

    if (deckOpenLocation == CANCELLED) {
        return;
    }

    if (deckOpenLocation == NEW_TAB) {
        emit openDeckEditor(nullptr);
        return;
    }

    deckDockWidget->cleanDeck();
    setModified(false);
    deckMenu->setSaveStatus(false);
}

void TabDeckEditorVisual::actLoadDeck()
{
    auto deckOpenLocation = confirmOpen();

    if (deckOpenLocation == CANCELLED) {
        return;
    }

    DlgLoadDeck dialog(this);
    if (!dialog.exec())
        return;

    QString fileName = dialog.selectedFiles().at(0);
    TabDeckEditorVisual::openDeckFromFile(fileName, deckOpenLocation);
    deckDockWidget->updateBannerCardComboBox();
}

void TabDeckEditorVisual::openDeckFromFile(const QString &fileName, DeckOpenLocation deckOpenLocation)
{
    DeckLoader::FileFormat fmt = DeckLoader::getFormatFromName(fileName);

    auto *l = new DeckLoader;
    if (l->loadFromFile(fileName, fmt, true)) {
        SettingsCache::instance().recents().updateRecentlyOpenedDeckPaths(fileName);
        deckDockWidget->updateBannerCardComboBox();
        if (!l->getBannerCard().first.isEmpty()) {
            deckDockWidget->bannerCardComboBox->setCurrentIndex(
                deckDockWidget->bannerCardComboBox->findText(l->getBannerCard().first));
        }
        if (deckOpenLocation == NEW_TAB) {
            emit openDeckEditor(l);
        } else {
            deckMenu->setSaveStatus(false);
            setDeck(l);
        }
    } else {
        delete l;
        QMessageBox::critical(this, tr("Error"), tr("Could not open deck at %1").arg(fileName));
    }
    deckMenu->setSaveStatus(true);
    tabContainer->visualDeckView->updateDisplay();
}

void TabDeckEditorVisual::showPrintingSelector()
{
    printingSelectorDockWidget->printingSelector->setCard(cardInfoDockWidget->cardInfo->getInfo(), DECK_ZONE_MAIN);
    printingSelectorDockWidget->printingSelector->updateDisplay();
    aPrintingSelectorDockVisible->setChecked(true);
    printingSelectorDockWidget->setVisible(true);
}

// Method uses to sync docks state with menu items state
bool TabDeckEditorVisual::eventFilter(QObject *o, QEvent *e)
{
    if (e->type() == QEvent::Close) {
        if (o == cardInfoDockWidget) {
            aCardInfoDockVisible->setChecked(false);
            aCardInfoDockFloating->setEnabled(false);
        } else if (o == deckDockWidget) {
            aDeckDockVisible->setChecked(false);
            aDeckDockFloating->setEnabled(false);
        } else if (o == deckAnalyticsDock) {
            aDeckAnalyticsDockVisible->setChecked(false);
            aDeckAnalyticsDockFloating->setEnabled(false);
        } else if (o == filterDockWidget) {
            aFilterDockVisible->setChecked(false);
            aFilterDockFloating->setEnabled(false);
        }
    }
    if (o == this && e->type() == QEvent::Hide) {
        LayoutsSettings &layouts = SettingsCache::instance().layouts();
        layouts.setDeckEditorLayoutState(saveState());
        layouts.setDeckEditorGeometry(saveGeometry());
        layouts.setDeckEditorCardSize(cardInfoDockWidget->size());
        layouts.setDeckEditorFilterSize(filterDockWidget->size());
        layouts.setDeckEditorDeckSize(deckDockWidget->size());
    }
    return false;
}

void TabDeckEditorVisual::dockVisibleTriggered()
{
    QObject *o = sender();
    if (o == aCardInfoDockVisible) {
        cardInfoDockWidget->setVisible(aCardInfoDockVisible->isChecked());
        aCardInfoDockFloating->setEnabled(aCardInfoDockVisible->isChecked());
        return;
    }

    if (o == aDeckDockVisible) {
        deckDockWidget->setVisible(aDeckDockVisible->isChecked());
        aDeckDockFloating->setEnabled(aDeckDockVisible->isChecked());
        return;
    }

    if (o == aDeckAnalyticsDockVisible) {
        deckAnalyticsDock->setVisible(aDeckAnalyticsDockVisible->isChecked());
        aDeckAnalyticsDockFloating->setEnabled(aDeckAnalyticsDockVisible->isChecked());
        return;
    }

    if (o == aFilterDockVisible) {
        filterDockWidget->setVisible(aFilterDockVisible->isChecked());
        aFilterDockFloating->setEnabled(aFilterDockVisible->isChecked());
        return;
    }
}

void TabDeckEditorVisual::dockFloatingTriggered()
{
    QObject *o = sender();
    if (o == aCardInfoDockFloating) {
        cardInfoDockWidget->setFloating(aCardInfoDockFloating->isChecked());
        return;
    }

    if (o == aDeckDockFloating) {
        deckDockWidget->setFloating(aDeckDockFloating->isChecked());
        return;
    }

    if (o == aDeckAnalyticsDockFloating) {
        deckAnalyticsDock->setFloating(aDeckAnalyticsDockFloating->isChecked());
        return;
    }

    if (o == aFilterDockFloating) {
        filterDockWidget->setFloating(aFilterDockFloating->isChecked());
        return;
    }
}

void TabDeckEditorVisual::dockTopLevelChanged(bool topLevel)
{
    QObject *o = sender();
    if (o == cardInfoDockWidget) {
        aCardInfoDockFloating->setChecked(topLevel);
        return;
    }

    if (o == deckDockWidget) {
        aDeckDockFloating->setChecked(topLevel);
        return;
    }

    if (o == filterDockWidget) {
        aFilterDockFloating->setChecked(topLevel);
        return;
    }

    if (o == deckAnalyticsDock) {
        aDeckAnalyticsDockFloating->setChecked(topLevel);
    }
}