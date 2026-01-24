#include "tab_deck_editor_visual.h"

#include "../../../../client/settings/cache_settings.h"
#include "../../deck_editor/deck_state_manager.h"
#include "../../filters/filter_builder.h"
#include "../../interface/pixel_map_generator.h"
#include "../../interface/widgets/cards/card_info_frame_widget.h"
#include "../../interface/widgets/deck_analytics/deck_analytics_widget.h"
#include "../../interface/widgets/visual_deck_editor/visual_deck_editor_widget.h"
#include "../tab_deck_editor.h"
#include "../tab_supervisor.h"
#include "tab_deck_editor_visual_tab_widget.h"

#include <QAction>
#include <QCloseEvent>
#include <QCompleter>
#include <QDir>
#include <QDockWidget>
#include <QHeaderView>
#include <QLineEdit>
#include <QMenu>
#include <QProcessEnvironment>
#include <QSplitter>
#include <QTextStream>
#include <QTimer>
#include <QTreeView>
#include <QVBoxLayout>
#include <libcockatrice/models/deck_list/deck_list_model.h>
#include <libcockatrice/protocol/pb/command_deck_upload.pb.h>
#include <libcockatrice/protocol/pending_command.h>

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
    cardDatabaseDockWidget->setHidden(true);
}

/** @brief Creates the central frame containing the tab container. */
void TabDeckEditorVisual::createCentralFrame()
{
    centralWidget = new QWidget(this);
    centralWidget->setObjectName("centralWidget");

    centralFrame = new QVBoxLayout;
    centralWidget->setLayout(centralFrame);

    tabContainer = new TabDeckEditorVisualTabWidget(
        centralWidget, this, deckStateManager->getModel(), cardDatabaseDockWidget->databaseDisplayWidget->databaseModel,
        cardDatabaseDockWidget->databaseDisplayWidget->databaseDisplayModel);

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
    tabContainer->deckAnalytics->updateDisplays();
    tabContainer->sampleHandWidget->setDeckModel(deckStateManager->getModel());
}

/** @brief Creates menus for deck editing and view options, including dock actions. */
void TabDeckEditorVisual::createMenus()
{
    deckMenu = new DeckEditorMenu(this);
    addTabMenu(deckMenu);

    viewMenu = new QMenu(this);

    registerDockWidget(viewMenu, cardInfoDockWidget);
    registerDockWidget(viewMenu, deckDockWidget);
    registerDockWidget(viewMenu, filterDockWidget);
    registerDockWidget(viewMenu, printingSelectorDockWidget);

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
    QString result = tr("Visual Deck: %1").arg(deckStateManager->getSimpleDeckName());
    if (deckStateManager->isModified())
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
    QModelIndex index = deckStateManager->getModel()->findCard(cardName, DECK_ZONE_MAIN);
    if (!index.isValid()) {
        index = deckStateManager->getModel()->findCard(cardName, DECK_ZONE_SIDE);
    }
    if (!deckDockWidget->getSelectionModel()->hasSelection()) {
        deckDockWidget->getSelectionModel()->setCurrentIndex(index, QItemSelectionModel::NoUpdate);
    }
}

void TabDeckEditorVisual::processMainboardCardClick(QMouseEvent *event,
                                                    CardInfoPictureWithTextOverlayWidget *instance,
                                                    const QString &zoneName)
{
    auto card = instance->getCard();

    // Get the model index for the card
    QModelIndex idx = deckStateManager->getModel()->findCard(card.getName(), zoneName);
    if (!idx.isValid()) {
        return;
    }

    QItemSelectionModel *sel = deckDockWidget->getSelectionModel();

    // Double click = swap
    if (event->type() == QEvent::MouseButtonDblClick && event->button() == Qt::LeftButton) {
        deckStateManager->swapCardAtIndex(idx);
        idx = deckStateManager->getModel()->findCard(card.getName(), zoneName);
        sel->setCurrentIndex(idx, QItemSelectionModel::ClearAndSelect);
        return;
    }

    // Right-click = decrement
    if (event->button() == Qt::RightButton) {
        actDecrementCard(card);
        //  Keep selection intact.
        return;
    }

    // Alt + Left click = increment
    if (event->button() == Qt::LeftButton && event->modifiers().testFlag(Qt::AltModifier)) {
        // actIncrementCard(card);
        //  Keep selection intact.
        return;
    }

    // Normal selection behavior
    if (event->button() == Qt::LeftButton) {
        Qt::KeyboardModifiers mods = event->modifiers();
        QItemSelectionModel::SelectionFlags flags;

        if (mods.testFlag(Qt::ControlModifier)) {
            // CTRL + click = toggle selection
            flags = QItemSelectionModel::Toggle;
        } else if (mods.testFlag(Qt::ShiftModifier)) {
            // SHIFT + click = select range
            QModelIndex anchor = sel->currentIndex();
            if (!anchor.isValid()) {
                anchor = idx;
            }

            QItemSelection range(anchor, idx);
            sel->select(range, QItemSelectionModel::SelectCurrent);
            sel->setCurrentIndex(idx, QItemSelectionModel::NoUpdate);
            return;
        } else {
            // Normal click = clear selection, select this, set current
            deckDockWidget->deckView->setCurrentIndex(idx);
            deckDockWidget->deckView->scrollTo(idx);
            return;
        }

        sel->setCurrentIndex(idx, flags);
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
    printingSelectorDockWidget->printingSelector->setCard(cardInfoDockWidget->cardInfo->getCard().getCardPtr());
    printingSelectorDockWidget->printingSelector->updateDisplay();
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
    for (auto dockWidget : dockToActions.keys()) {
        dockWidget->setFloating(false);
    }

    deckDockWidget->setVisible(true);
    cardInfoDockWidget->setVisible(true);
    filterDockWidget->setVisible(false);
    printingSelectorDockWidget->setVisible(true);

    setCentralWidget(centralWidget);
    addDockWidget(Qt::RightDockWidgetArea, deckDockWidget);
    addDockWidget(Qt::RightDockWidgetArea, cardInfoDockWidget);
    addDockWidget(Qt::RightDockWidgetArea, filterDockWidget);
    addDockWidget(Qt::RightDockWidgetArea, printingSelectorDockWidget);

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

    dockToActions[cardInfoDockWidget].menu->setTitle(tr("Card Info"));
    dockToActions[deckDockWidget].menu->setTitle(tr("Deck"));
    dockToActions[filterDockWidget].menu->setTitle(tr("Filters"));
    dockToActions[printingSelectorDockWidget].menu->setTitle(tr("Printing"));

    for (auto &actions : dockToActions.values()) {
        actions.aVisible->setText(tr("Visible"));
        actions.aFloating->setText(tr("Floating"));
    }

    aResetLayout->setText(tr("Reset layout"));
}

/**
 * @brief Handles dock visibility, floating, and layout saving events.
 *
 * Keeps dock state in sync with menu items and saves layout when hidden.
 */
bool TabDeckEditorVisual::eventFilter(QObject *o, QEvent *e)
{
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
