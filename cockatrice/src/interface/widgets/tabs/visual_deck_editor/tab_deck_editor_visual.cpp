#include "tab_deck_editor_visual.h"

#include "../../../../client/settings/cache_settings.h"
#include "../../client/network/interfaces/deck_stats_interface.h"
#include "../../filters/filter_builder.h"
#include "../../interface/pixel_map_generator.h"
#include "../../interface/widgets/cards/card_info_frame_widget.h"
#include "../../interface/widgets/deck_analytics/deck_analytics_widget.h"
#include "../../interface/widgets/visual_deck_editor/visual_deck_editor_widget.h"
#include "../tab_deck_editor.h"
#include "../tab_supervisor.h"
#include "tab_deck_editor_visual_tab_widget.h"

#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QCompleter>
#include <QDir>
#include <QDockWidget>
#include <QFileDialog>
#include <QHeaderView>
#include <QLineEdit>
#include <QMenu>
#include <QPrintPreviewDialog>
#include <QProcessEnvironment>
#include <QSplitter>
#include <QTextStream>
#include <QTimer>
#include <QTreeView>
#include <QVBoxLayout>
#include <libcockatrice/models/database/card_database_model.h>
#include <libcockatrice/models/deck_list/deck_list_model.h>
#include <libcockatrice/protocol/pb/command_deck_upload.pb.h>
#include <libcockatrice/protocol/pending_command.h>
#include <libcockatrice/utility/trice_limits.h>

/**
 * @brief Constructs the TabDeckEditorVisual instance.
 *
 * Sets up the central widget, tab container, menus, shortcuts,
 * and restores the saved layout.
 * @param _tabSupervisor Parent tab supervisor managing this tab.
 */
TabDeckEditorVisual::TabDeckEditorVisual(TabSupervisor *_tabSupervisor) : AbstractTabDeckEditor(_tabSupervisor)
{
    setObjectName("TabDeckEditorVisual");

    createCentralFrame();
    createMenus();

    installEventFilter(this);

    retranslateUi();
    connect(&SettingsCache::instance().shortcuts(), SIGNAL(shortCutChanged()), this, SLOT(refreshShortcuts()));
    refreshShortcuts();

    loadLayout();
    databaseDisplayDockWidget->setHidden(true);
}

/** @brief Creates the central frame containing the tab container. */
void TabDeckEditorVisual::createCentralFrame()
{
    centralWidget = new QWidget(this);
    centralWidget->setObjectName("centralWidget");

    centralFrame = new QVBoxLayout;
    centralWidget->setLayout(centralFrame);

    tabContainer = new TabDeckEditorVisualTabWidget(centralWidget, this, deckDockWidget->deckModel,
                                                    databaseDisplayDockWidget->databaseModel,
                                                    databaseDisplayDockWidget->databaseDisplayModel);

    connect(tabContainer, &TabDeckEditorVisualTabWidget::cardChanged, this,
            &TabDeckEditorVisual::changeModelIndexAndCardInfo);
    connect(tabContainer, &TabDeckEditorVisualTabWidget::cardChangedDatabaseDisplay, this,
            &AbstractTabDeckEditor::updateCard);
    connect(tabContainer, &TabDeckEditorVisualTabWidget::cardClicked, this,
            &TabDeckEditorVisual::processMainboardCardClick);
    connect(tabContainer, &TabDeckEditorVisualTabWidget::cardClickedDatabaseDisplay, this,
            &TabDeckEditorVisual::processCardClickDatabaseDisplay);

    centralFrame->addWidget(tabContainer);
    setCentralWidget(centralWidget);
    setDockOptions(QMainWindow::AnimatedDocks | QMainWindow::AllowNestedDocks | QMainWindow::AllowTabbedDocks);
}

/** @brief Updates the visual deck, analytics, and sample hand after a deck change. */
void TabDeckEditorVisual::onDeckChanged()
{
    AbstractTabDeckEditor::onDeckModified();
    tabContainer->visualDeckView->constructZoneWidgetsFromDeckListModel();
    tabContainer->deckAnalytics->refreshDisplays(deckDockWidget->deckModel);
    tabContainer->sampleHandWidget->setDeckModel(deckDockWidget->deckModel);
}

/** @brief Creates menus for deck editing and view options, including dock actions. */
void TabDeckEditorVisual::createMenus()
{
    deckMenu = new DeckEditorMenu(this);
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

    if (SettingsCache::instance().getOverrideAllCardArtWithPersonalPreference()) {
        printingSelectorDockMenu->setEnabled(false);
    }

    connect(&SettingsCache::instance(), &SettingsCache::overrideAllCardArtWithPersonalPreferenceChanged, this,
            [this](bool enabled) { printingSelectorDockMenu->setEnabled(!enabled); });

    viewMenu->addSeparator();

    aResetLayout = viewMenu->addAction(QString());
    connect(aResetLayout, SIGNAL(triggered()), this, SLOT(restartLayout()));
    viewMenu->addAction(aResetLayout);

    deckMenu->setSaveStatus(false);
    addTabMenu(viewMenu);
}

/** @brief Returns the tab text, prepending a mark if the deck has unsaved changes. */
QString TabDeckEditorVisual::getTabText() const
{
    QString result = tr("Visual Deck: %1").arg(deckDockWidget->getSimpleDeckName());
    if (modified)
        result.prepend("* ");
    return result;
}

/** @brief Updates card info and highlights the corresponding card in the deck view. */
void TabDeckEditorVisual::changeModelIndexAndCardInfo(const ExactCard &activeCard)
{
    updateCard(activeCard);
    changeModelIndexToCard(activeCard);
}

/** @brief Selects the given card in the deck view, checking main and side zones. */
void TabDeckEditorVisual::changeModelIndexToCard(const ExactCard &activeCard)
{
    QString cardName = activeCard.getName();
    QModelIndex index = deckDockWidget->deckModel->findCard(cardName, DECK_ZONE_MAIN);
    if (!index.isValid()) {
        index = deckDockWidget->deckModel->findCard(cardName, DECK_ZONE_SIDE);
    }
    deckDockWidget->deckView->setCurrentIndex(index);
}

/** @brief Handles clicks on cards in the mainboard deck. */
void TabDeckEditorVisual::processMainboardCardClick(QMouseEvent *event,
                                                    CardInfoPictureWithTextOverlayWidget *instance,
                                                    QString zoneName)
{
    if (event->button() == Qt::LeftButton) {
        actSwapCard(instance->getCard(), zoneName);
    } else if (event->button() == Qt::RightButton) {
        actDecrementCard(instance->getCard());
    } else if (event->button() == Qt::MiddleButton) {
        deckDockWidget->actRemoveCard();
    }
}

/** @brief Handles clicks on cards in the database display. */
void TabDeckEditorVisual::processCardClickDatabaseDisplay(QMouseEvent *event,
                                                          CardInfoPictureWithTextOverlayWidget *instance)
{
    if (event->button() == Qt::LeftButton) {
        actAddCard(instance->getCard());
    } else if (event->button() == Qt::RightButton) {
        actDecrementCard(instance->getCard());
    } else if (event->button() == Qt::MiddleButton) {
        deckDockWidget->actRemoveCard();
    }
}

/** @brief Performs "Save Deck As..." while temporarily disabling the search bar. */
bool TabDeckEditorVisual::actSaveDeckAs()
{
    tabContainer->visualDeckView->searchBar->setEnabled(false);
    auto result = AbstractTabDeckEditor::actSaveDeckAs();
    tabContainer->visualDeckView->searchBar->setEnabled(true);
    return result;
}

/** @brief Shows the printing selector dock and updates it with the current card. */
void TabDeckEditorVisual::showPrintingSelector()
{
    printingSelectorDockWidget->printingSelector->setCard(cardInfoDockWidget->cardInfo->getCard().getCardPtr(),
                                                          DECK_ZONE_MAIN);
    printingSelectorDockWidget->printingSelector->updateDisplay();
    aPrintingSelectorDockVisible->setChecked(true);
    printingSelectorDockWidget->setVisible(true);
}

/** @brief Set size restrictions for free floating dock widgets. */
void TabDeckEditorVisual::freeDocksSize()
{
    deckDockWidget->setMinimumSize(100, 100);
    deckDockWidget->setMaximumSize(5000, 5000);

    cardInfoDockWidget->setMinimumSize(100, 100);
    cardInfoDockWidget->setMaximumSize(5000, 5000);

    filterDockWidget->setMinimumSize(100, 100);
    filterDockWidget->setMaximumSize(5000, 5000);

    printingSelectorDockWidget->setMinimumSize(100, 100);
    printingSelectorDockWidget->setMaximumSize(5000, 5000);
}

/** @brief Refreshes keyboard shortcuts for this tab from settings. */
void TabDeckEditorVisual::refreshShortcuts()
{
    ShortcutsSettings &shortcuts = SettingsCache::instance().shortcuts();
    aResetLayout->setShortcuts(shortcuts.getShortcut("TabDeckEditorVisual/aResetLayout"));
}

/** @brief Loads the saved layout or resets to default if no layout exists. */
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

    if (SettingsCache::instance().getOverrideAllCardArtWithPersonalPreference()) {
        if (!printingSelectorDockWidget->isHidden()) {
            printingSelectorDockWidget->setHidden(true);
            aPrintingSelectorDockVisible->setChecked(false);
        }
    }

    aCardInfoDockVisible->setChecked(!cardInfoDockWidget->isHidden());
    aFilterDockVisible->setChecked(!filterDockWidget->isHidden());
    aDeckDockVisible->setChecked(!deckDockWidget->isHidden());
    aPrintingSelectorDockVisible->setChecked(!printingSelectorDockWidget->isHidden());

    aCardInfoDockFloating->setEnabled(aCardInfoDockVisible->isChecked());
    aDeckDockFloating->setEnabled(aDeckDockVisible->isChecked());
    aFilterDockFloating->setEnabled(aFilterDockVisible->isChecked());
    aPrintingSelectorDockFloating->setEnabled(aPrintingSelectorDockVisible->isChecked());

    aCardInfoDockFloating->setChecked(cardInfoDockWidget->isFloating());
    aFilterDockFloating->setChecked(filterDockWidget->isFloating());
    aDeckDockFloating->setChecked(deckDockWidget->isFloating());
    aPrintingSelectorDockFloating->setChecked(printingSelectorDockWidget->isFloating());

    cardInfoDockWidget->setMinimumSize(layouts.getDeckEditorCardSize());
    cardInfoDockWidget->setMaximumSize(layouts.getDeckEditorCardSize());

    filterDockWidget->setMinimumSize(layouts.getDeckEditorFilterSize());
    filterDockWidget->setMaximumSize(layouts.getDeckEditorFilterSize());

    deckDockWidget->setMinimumSize(layouts.getDeckEditorDeckSize());
    deckDockWidget->setMaximumSize(layouts.getDeckEditorDeckSize());

    printingSelectorDockWidget->setMinimumSize(layouts.getDeckEditorPrintingSelectorSize());
    printingSelectorDockWidget->setMaximumSize(layouts.getDeckEditorPrintingSelectorSize());

    QTimer::singleShot(100, this, &TabDeckEditorVisual::freeDocksSize);
}

/** @brief Resets the layout to default positions and dock states. */
void TabDeckEditorVisual::restartLayout()
{
    aCardInfoDockVisible->setChecked(true);
    aDeckDockVisible->setChecked(true);
    aFilterDockVisible->setChecked(false);
    aPrintingSelectorDockVisible->setChecked(!SettingsCache::instance().getOverrideAllCardArtWithPersonalPreference());

    aCardInfoDockFloating->setChecked(false);
    aDeckDockFloating->setChecked(false);
    aFilterDockFloating->setChecked(false);
    aPrintingSelectorDockFloating->setChecked(false);

    setCentralWidget(centralWidget);
    addDockWidget(Qt::RightDockWidgetArea, deckDockWidget);
    addDockWidget(Qt::RightDockWidgetArea, cardInfoDockWidget);
    addDockWidget(Qt::RightDockWidgetArea, filterDockWidget);
    addDockWidget(Qt::RightDockWidgetArea, printingSelectorDockWidget);

    deckDockWidget->setVisible(true);
    cardInfoDockWidget->setVisible(true);
    filterDockWidget->setVisible(false);
    printingSelectorDockWidget->setVisible(!SettingsCache::instance().getOverrideAllCardArtWithPersonalPreference());

    deckDockWidget->setFloating(false);
    cardInfoDockWidget->setFloating(false);
    filterDockWidget->setFloating(false);
    printingSelectorDockWidget->setFloating(false);

    splitDockWidget(cardInfoDockWidget, printingSelectorDockWidget, Qt::Vertical);
    splitDockWidget(cardInfoDockWidget, deckDockWidget, Qt::Horizontal);
    splitDockWidget(cardInfoDockWidget, filterDockWidget, Qt::Horizontal);

    QTimer::singleShot(100, this, SLOT(freeDocksSize()));
}

/** @brief Retranslates UI elements for localization. */
void TabDeckEditorVisual::retranslateUi()
{
    deckMenu->setTitle(tr("&Visual Deck Editor"));

    cardInfoDockWidget->setWindowTitle(tr("Card Info"));
    deckDockWidget->setWindowTitle(tr("Deck"));
    filterDockWidget->setWindowTitle(tr("Filters"));

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

/**
 * @brief Handles dock visibility, floating, and layout saving events.
 *
 * Keeps dock state in sync with menu items and saves layout when hidden.
 */
bool TabDeckEditorVisual::eventFilter(QObject *o, QEvent *e)
{
    if (e->type() == QEvent::Close) {
        if (o == cardInfoDockWidget) {
            aCardInfoDockVisible->setChecked(false);
            aCardInfoDockFloating->setEnabled(false);
        } else if (o == deckDockWidget) {
            aDeckDockVisible->setChecked(false);
            aDeckDockFloating->setEnabled(false);
        } else if (o == filterDockWidget) {
            aFilterDockVisible->setChecked(false);
            aFilterDockFloating->setEnabled(false);
        } else if (o == printingSelectorDockWidget) {
            aPrintingSelectorDockVisible->setChecked(false);
            aPrintingSelectorDockFloating->setEnabled(false);
        }
    }

    if (o == this && e->type() == QEvent::Hide) {
        LayoutsSettings &layouts = SettingsCache::instance().layouts();
        layouts.setDeckEditorLayoutState(saveState());
        layouts.setDeckEditorGeometry(saveGeometry());
        layouts.setDeckEditorCardSize(cardInfoDockWidget->size());
        layouts.setDeckEditorFilterSize(filterDockWidget->size());
        layouts.setDeckEditorDeckSize(deckDockWidget->size());
        layouts.setDeckEditorPrintingSelectorSize(printingSelectorDockWidget->size());
    }
    return false;
}

/** @brief Toggles dock visibility based on the corresponding menu action. */
void TabDeckEditorVisual::dockVisibleTriggered()
{
    QObject *o = sender();
    if (o == aCardInfoDockVisible) {
        cardInfoDockWidget->setHidden(!aCardInfoDockVisible->isChecked());
        aCardInfoDockFloating->setEnabled(aCardInfoDockVisible->isChecked());
        return;
    }

    if (o == aDeckDockVisible) {
        deckDockWidget->setHidden(!aDeckDockVisible->isChecked());
        aDeckDockFloating->setEnabled(aDeckDockVisible->isChecked());
        return;
    }

    if (o == aFilterDockVisible) {
        filterDockWidget->setHidden(!aFilterDockVisible->isChecked());
        aFilterDockFloating->setEnabled(aFilterDockVisible->isChecked());
        return;
    }

    if (o == aPrintingSelectorDockVisible) {
        printingSelectorDockWidget->setHidden(!aPrintingSelectorDockVisible->isChecked());
        aPrintingSelectorDockFloating->setEnabled(aPrintingSelectorDockVisible->isChecked());
        return;
    }
}

/** @brief Toggles dock floating state based on the corresponding menu action. */
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

    if (o == aFilterDockFloating) {
        filterDockWidget->setFloating(aFilterDockFloating->isChecked());
        return;
    }

    if (o == aPrintingSelectorDockFloating) {
        printingSelectorDockWidget->setFloating(aPrintingSelectorDockFloating->isChecked());
        return;
    }
}

/** @brief Updates menu checkboxes when a dock's top-level/floating state changes. */
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

    if (o == printingSelectorDockWidget) {
        aPrintingSelectorDockFloating->setChecked(topLevel);
        return;
    }
}
