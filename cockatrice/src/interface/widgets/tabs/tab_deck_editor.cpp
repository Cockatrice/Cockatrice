#include "tab_deck_editor.h"

#include "../../../client/settings/cache_settings.h"
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

    cardInfoDockMenu = viewMenu->addMenu(QString());
    deckDockMenu = viewMenu->addMenu(QString());
    filterDockMenu = viewMenu->addMenu(QString());
    printingSelectorDockMenu = viewMenu->addMenu(QString());

    // Card Info dock
    aCardInfoDockVisible = cardInfoDockMenu->addAction(QString());
    aCardInfoDockVisible->setCheckable(true);
    connect(aCardInfoDockVisible, &QAction::triggered, this, &TabDeckEditor::dockVisibleTriggered);
    aCardInfoDockFloating = cardInfoDockMenu->addAction(QString());
    aCardInfoDockFloating->setCheckable(true);
    connect(aCardInfoDockFloating, &QAction::triggered, this, &TabDeckEditor::dockFloatingTriggered);

    // Deck dock
    aDeckDockVisible = deckDockMenu->addAction(QString());
    aDeckDockVisible->setCheckable(true);
    connect(aDeckDockVisible, &QAction::triggered, this, &TabDeckEditor::dockVisibleTriggered);
    aDeckDockFloating = deckDockMenu->addAction(QString());
    aDeckDockFloating->setCheckable(true);
    connect(aDeckDockFloating, &QAction::triggered, this, &TabDeckEditor::dockFloatingTriggered);

    // Filter dock
    aFilterDockVisible = filterDockMenu->addAction(QString());
    aFilterDockVisible->setCheckable(true);
    connect(aFilterDockVisible, &QAction::triggered, this, &TabDeckEditor::dockVisibleTriggered);
    aFilterDockFloating = filterDockMenu->addAction(QString());
    aFilterDockFloating->setCheckable(true);
    connect(aFilterDockFloating, &QAction::triggered, this, &TabDeckEditor::dockFloatingTriggered);

    // Printing selector dock
    aPrintingSelectorDockVisible = printingSelectorDockMenu->addAction(QString());
    aPrintingSelectorDockVisible->setCheckable(true);
    connect(aPrintingSelectorDockVisible, &QAction::triggered, this, &TabDeckEditor::dockVisibleTriggered);
    aPrintingSelectorDockFloating = printingSelectorDockMenu->addAction(QString());
    aPrintingSelectorDockFloating->setCheckable(true);
    connect(aPrintingSelectorDockFloating, &QAction::triggered, this, &TabDeckEditor::dockFloatingTriggered);

    if (SettingsCache::instance().getOverrideAllCardArtWithPersonalPreference()) {
        printingSelectorDockMenu->setEnabled(false);
    }

    connect(&SettingsCache::instance(), &SettingsCache::overrideAllCardArtWithPersonalPreferenceChanged, this,
            [this](bool enabled) { printingSelectorDockMenu->setEnabled(!enabled); });

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
    QString result = tr("Deck: %1").arg(deckDockWidget->getSimpleDeckName());
    if (modified)
        result.prepend("* ");
    return result;
}

/** @brief Updates text of menus and actions for localization. */
void TabDeckEditor::retranslateUi()
{
    deckMenu->retranslateUi();
    cardInfoDockWidget->retranslateUi();
    deckDockWidget->retranslateUi();
    filterDockWidget->retranslateUi();
    printingSelectorDockWidget->retranslateUi();

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
    printingSelectorDockWidget->printingSelector->setCard(cardInfoDockWidget->cardInfo->getCard().getCardPtr(),
                                                          DECK_ZONE_MAIN);
    printingSelectorDockWidget->printingSelector->updateDisplay();
    aPrintingSelectorDockVisible->setChecked(true);
    printingSelectorDockWidget->setVisible(true);
}

/**
 * @brief Loads deck editor layout from settings or resets to default.
 */
void TabDeckEditor::loadLayout()
{
    LayoutsSettings &layouts = SettingsCache::instance().layouts();
    setCentralWidget(databaseDisplayDockWidget);

    auto &layoutState = layouts.getDeckEditorLayoutState();
    if (layoutState.isNull())
        restartLayout();
    else {
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

    QTimer::singleShot(100, this, &TabDeckEditor::freeDocksSize);
}

/**
 * @brief Resets all dock widgets to default layout.
 */
void TabDeckEditor::restartLayout()
{

    // Update menu checkboxes
    aCardInfoDockVisible->setChecked(true);
    aDeckDockVisible->setChecked(true);
    aFilterDockVisible->setChecked(true);
    aPrintingSelectorDockVisible->setChecked(!SettingsCache::instance().getOverrideAllCardArtWithPersonalPreference());

    aCardInfoDockFloating->setChecked(false);
    aDeckDockFloating->setChecked(false);
    aFilterDockFloating->setChecked(false);
    aPrintingSelectorDockFloating->setChecked(false);

    setCentralWidget(databaseDisplayDockWidget);
    addDockWidget(Qt::RightDockWidgetArea, deckDockWidget);
    addDockWidget(Qt::RightDockWidgetArea, cardInfoDockWidget);
    addDockWidget(Qt::RightDockWidgetArea, filterDockWidget);
    addDockWidget(Qt::RightDockWidgetArea, printingSelectorDockWidget);

    // Show/hide and reset floating
    deckDockWidget->setFloating(false);
    cardInfoDockWidget->setFloating(false);
    filterDockWidget->setFloating(false);
    printingSelectorDockWidget->setFloating(false);

    deckDockWidget->setVisible(true);
    cardInfoDockWidget->setVisible(true);
    filterDockWidget->setVisible(true);
    printingSelectorDockWidget->setVisible(!SettingsCache::instance().getOverrideAllCardArtWithPersonalPreference());

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

    deckDockWidget->setMinimumSize(minSize);
    deckDockWidget->setMaximumSize(maxSize);

    cardInfoDockWidget->setMinimumSize(minSize);
    cardInfoDockWidget->setMaximumSize(maxSize);

    filterDockWidget->setMinimumSize(minSize);
    filterDockWidget->setMaximumSize(maxSize);

    printingSelectorDockWidget->setMinimumSize(minSize);
    printingSelectorDockWidget->setMaximumSize(maxSize);
}

/** @brief Handles dock visibility toggling from menu actions. */
void TabDeckEditor::dockVisibleTriggered()
{
    QObject *o = sender();
    if (o == aCardInfoDockVisible) {
        cardInfoDockWidget->setHidden(!aCardInfoDockVisible->isChecked());
        aCardInfoDockFloating->setEnabled(aCardInfoDockVisible->isChecked());
    } else if (o == aDeckDockVisible) {
        deckDockWidget->setHidden(!aDeckDockVisible->isChecked());
        aDeckDockFloating->setEnabled(aDeckDockVisible->isChecked());
    } else if (o == aFilterDockVisible) {
        filterDockWidget->setHidden(!aFilterDockVisible->isChecked());
        aFilterDockFloating->setEnabled(aFilterDockVisible->isChecked());
    } else if (o == aPrintingSelectorDockVisible) {
        printingSelectorDockWidget->setHidden(!aPrintingSelectorDockVisible->isChecked());
        aPrintingSelectorDockFloating->setEnabled(aPrintingSelectorDockVisible->isChecked());
    }
}

/** @brief Handles dock floating toggling from menu actions. */
void TabDeckEditor::dockFloatingTriggered()
{
    QObject *o = sender();
    if (o == aCardInfoDockFloating)
        cardInfoDockWidget->setFloating(aCardInfoDockFloating->isChecked());
    else if (o == aDeckDockFloating)
        deckDockWidget->setFloating(aDeckDockFloating->isChecked());
    else if (o == aFilterDockFloating)
        filterDockWidget->setFloating(aFilterDockFloating->isChecked());
    else if (o == aPrintingSelectorDockFloating)
        printingSelectorDockWidget->setFloating(aPrintingSelectorDockFloating->isChecked());
}

/** @brief Syncs menu state with dock floating changes. */
void TabDeckEditor::dockTopLevelChanged(bool topLevel)
{
    QObject *o = sender();
    if (o == cardInfoDockWidget)
        aCardInfoDockFloating->setChecked(topLevel);
    else if (o == deckDockWidget)
        aDeckDockFloating->setChecked(topLevel);
    else if (o == filterDockWidget)
        aFilterDockFloating->setChecked(topLevel);
    else if (o == printingSelectorDockWidget)
        aPrintingSelectorDockFloating->setChecked(topLevel);
}

/**
 * @brief Handles close/hide events to update menu state and save layout.
 * @param o Object sending the event.
 * @param e Event.
 * @return Always returns false.
 */
bool TabDeckEditor::eventFilter(QObject *o, QEvent *e)
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
