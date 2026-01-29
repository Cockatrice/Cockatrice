#include "tab_deck_editor.h"

#include "../../../client/settings/cache_settings.h"
#include "../deck_editor/deck_state_manager.h"
#include "../filters/filter_builder.h"
#include "../interface/pixel_map_generator.h"
#include "../interface/widgets/cards/card_info_frame_widget.h"
#include "../interface/widgets/deck_editor/deck_editor_filter_dock_widget.h"
#include "../interface/widgets/menus/deck_editor_menu.h"
#include "tab_supervisor.h"

#include <QAction>
#include <QCloseEvent>
#include <QDebug>
#include <QDir>
#include <QDockWidget>
#include <QHeaderView>
#include <QMenuBar>
#include <QProcessEnvironment>
#include <QTimer>
#include <libcockatrice/models/database/card_database_model.h>
#include <libcockatrice/network/client/abstract/abstract_client.h>
#include <libcockatrice/protocol/pending_command.h>
#include <libcockatrice/utility/trice_limits.h>

/**
 * @brief Constructs a new TabDeckEditor object.
 * Sets up menus, translations, shortcuts, and loads the layout.
 * @param _tabSupervisor Pointer to parent TabSupervisor.
 */
TabDeckEditor::TabDeckEditor(TabSupervisor *_tabSupervisor) : AbstractTabDeckEditor(_tabSupervisor)
{
    setObjectName("TabDeckEditor");

    TabDeckEditor::createMenus();

    installEventFilter(this);

    TabDeckEditor::retranslateUi();
    TabDeckEditor::refreshShortcuts();

    TabDeckEditor::loadLayout();
}

/**
 * @brief Creates menus for deck editing and view customization.
 *
 * Adds DeckEditorMenu, view menus, and actions for dock visibility and floating.
 */
void TabDeckEditor::createMenus()
{
    deckMenu = new DeckEditorMenu(this);
    addTabMenu(deckMenu);

    viewMenu = new QMenu(this);

    registerDockWidget(viewMenu, cardDatabaseDockWidget);
    registerDockWidget(viewMenu, cardInfoDockWidget);
    registerDockWidget(viewMenu, deckDockWidget);
    registerDockWidget(viewMenu, filterDockWidget);
    registerDockWidget(viewMenu, printingSelectorDockWidget);

    connect(&SettingsCache::instance(), &SettingsCache::overrideAllCardArtWithPersonalPreferenceChanged, this,
            [this](bool enabled) { dockToActions[printingSelectorDockWidget].menu->setEnabled(!enabled); });

    viewMenu->addSeparator();

    aResetLayout = viewMenu->addAction(QString());
    connect(aResetLayout, &QAction::triggered, this, &TabDeckEditor::restartLayout);
    viewMenu->addAction(aResetLayout);

    deckMenu->setSaveStatus(false);
    addTabMenu(viewMenu);
}

/**
 * @brief Returns the text to display in the tab.
 * Prepends "*" if the deck is modified.
 * @return Tab text including modified mark.
 */
QString TabDeckEditor::getTabText() const
{
    QString result = tr("Deck: %1").arg(deckStateManager->getSimpleDeckName());
    if (deckStateManager->isModified())
        result.prepend("* ");
    return result;
}

/** @brief Updates text of menus and actions for localization. */
void TabDeckEditor::retranslateUi()
{
    deckMenu->retranslateUi();
    cardDatabaseDockWidget->retranslateUi();
    cardInfoDockWidget->retranslateUi();
    deckDockWidget->retranslateUi();
    filterDockWidget->retranslateUi();
    printingSelectorDockWidget->retranslateUi();

    viewMenu->setTitle(tr("&View"));

    dockToActions[cardDatabaseDockWidget].menu->setTitle(tr("Card Database"));
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

/** @brief Refreshes shortcuts for deck editor menu actions. */
void TabDeckEditor::refreshShortcuts()
{
    ShortcutsSettings &shortcuts = SettingsCache::instance().shortcuts();
    aResetLayout->setShortcuts(shortcuts.getShortcut("TabDeckEditor/aResetLayout"));
}

/**
 * @brief Displays the printing selector dock with the current card.
 */
void TabDeckEditor::showPrintingSelector()
{
    printingSelectorDockWidget->printingSelector->setCard(cardInfoDockWidget->cardInfo->getCard().getCardPtr());
    printingSelectorDockWidget->printingSelector->updateDisplay();
    printingSelectorDockWidget->setVisible(true);
}

/**
 * @brief Loads deck editor layout from settings or resets to default.
 */
void TabDeckEditor::loadLayout()
{
    LayoutsSettings &layouts = SettingsCache::instance().layouts();

    auto &layoutState = layouts.getDeckEditorLayoutState();
    if (layoutState.isNull())
        restartLayout();
    else {
        restoreState(layoutState);
        restoreGeometry(layouts.getDeckEditorGeometry());
    }

    cardDatabaseDockWidget->setMinimumSize(layouts.getDeckEditorCardDatabaseSize());
    cardDatabaseDockWidget->setMaximumSize(layouts.getDeckEditorCardDatabaseSize());

    cardInfoDockWidget->setMinimumSize(layouts.getDeckEditorCardSize());
    cardInfoDockWidget->setMaximumSize(layouts.getDeckEditorCardSize());

    filterDockWidget->setMinimumSize(layouts.getDeckEditorFilterSize());
    filterDockWidget->setMaximumSize(layouts.getDeckEditorFilterSize());

    deckDockWidget->setMinimumSize(layouts.getDeckEditorDeckSize());
    deckDockWidget->setMaximumSize(layouts.getDeckEditorDeckSize());

    printingSelectorDockWidget->setMinimumSize(layouts.getDeckEditorPrintingSelectorSize());
    printingSelectorDockWidget->setMaximumSize(layouts.getDeckEditorPrintingSelectorSize());

    QTimer::singleShot(100, this, &TabDeckEditor::freeDocksSize);
}

/**
 * @brief Resets all dock widgets to default layout.
 */
void TabDeckEditor::restartLayout()
{
    // Show/hide and reset floating
    for (auto dockWidget : dockToActions.keys()) {
        dockWidget->setVisible(true);
        dockWidget->setFloating(false);
    }

    addDockWidget(Qt::LeftDockWidgetArea, cardDatabaseDockWidget);
    addDockWidget(Qt::RightDockWidgetArea, deckDockWidget);
    addDockWidget(Qt::RightDockWidgetArea, cardInfoDockWidget);
    addDockWidget(Qt::RightDockWidgetArea, filterDockWidget);
    addDockWidget(Qt::RightDockWidgetArea, printingSelectorDockWidget);

    splitDockWidget(cardInfoDockWidget, printingSelectorDockWidget, Qt::Horizontal);
    splitDockWidget(printingSelectorDockWidget, deckDockWidget, Qt::Horizontal);
    splitDockWidget(cardInfoDockWidget, printingSelectorDockWidget, Qt::Horizontal);
    splitDockWidget(cardInfoDockWidget, filterDockWidget, Qt::Vertical);

    QTimer::singleShot(100, this, &TabDeckEditor::freeDocksSize);
}

/** @brief Frees dock sizes to allow flexible resizing. */
void TabDeckEditor::freeDocksSize()
{
    const QSize minSize(100, 100);
    const QSize maxSize(5000, 5000);

    for (auto dockWidget : dockToActions.keys()) {
        dockWidget->setMinimumSize(minSize);
        dockWidget->setMaximumSize(maxSize);
    }
}

/**
 * @brief Handles close/hide events to update menu state and save layout.
 * @param o Object sending the event.
 * @param e Event.
 * @return Always returns false.
 */
bool TabDeckEditor::eventFilter(QObject *o, QEvent *e)
{
    if (o == this && e->type() == QEvent::Hide) {
        LayoutsSettings &layouts = SettingsCache::instance().layouts();
        layouts.setDeckEditorLayoutState(saveState());
        layouts.setDeckEditorGeometry(saveGeometry());
        layouts.setDeckEditorCardDatabaseSize(cardDatabaseDockWidget->size());
        layouts.setDeckEditorCardSize(cardInfoDockWidget->size());
        layouts.setDeckEditorFilterSize(filterDockWidget->size());
        layouts.setDeckEditorDeckSize(deckDockWidget->size());
        layouts.setDeckEditorPrintingSelectorSize(printingSelectorDockWidget->size());
    }
    return false;
}
