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
#include "../menus/deck_editor/deck_editor_menu.h"
#include "../ui/pixel_map_generator.h"
#include "../ui/widgets/deck_editor/deck_editor_filter_dock_widget.h"
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

TabDeckEditor::TabDeckEditor(TabSupervisor *_tabSupervisor) : TabGenericDeckEditor(_tabSupervisor)
{
    setObjectName("TabDeckEditor");
    setDockOptions(QMainWindow::AnimatedDocks | QMainWindow::AllowNestedDocks | QMainWindow::AllowTabbedDocks);

    databaseDisplayDockWidget = new DeckEditorDatabaseDisplayWidget(this, this);
    deckDockWidget = new DeckEditorDeckDockWidget(this, this);
    cardInfoDockWidget = new DeckEditorCardInfoDockWidget(this);
    filterDockWidget = new DeckEditorFilterDockWidget(this, this);
    printingSelectorDockWidget = new DeckEditorPrintingSelectorDockWidget(this, this);

    TabDeckEditor::createMenus();

    this->installEventFilter(this);

    TabDeckEditor::retranslateUi();
    connect(&SettingsCache::instance().shortcuts(), SIGNAL(shortCutChanged()), this, SLOT(refreshShortcuts()));
    TabDeckEditor::refreshShortcuts();

    TabDeckEditor::loadLayout();
}

void TabDeckEditor::createMenus()
{
    deckMenu = new DeckEditorMenu(this, this);
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

    deckMenu->setSaveStatus(false);

    addTabMenu(viewMenu);
}

QString TabDeckEditor::getTabText() const
{
    QString result = tr("Deck: %1").arg(deckDockWidget->getSimpleDeckName());
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

    deckDockWidget->cleanDeck();
    setModified(false);
    deckMenu->setSaveStatus(false);
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
    deckDockWidget->updateBannerCardComboBox();
}

void TabDeckEditor::actLoadDeckFromClipboard()
{
    TabGenericDeckEditor::actLoadDeckFromClipboard();
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
}

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

void TabDeckEditor::refreshShortcuts()
{
    ShortcutsSettings &shortcuts = SettingsCache::instance().shortcuts();
    aResetLayout->setShortcuts(shortcuts.getShortcut("TabDeckEditor/aResetLayout"));
}

void TabDeckEditor::showPrintingSelector()
{
    printingSelectorDockWidget->printingSelector->setCard(cardInfoDockWidget->cardInfo->getInfo(), DECK_ZONE_MAIN);
    printingSelectorDockWidget->printingSelector->updateDisplay();
    aPrintingSelectorDockVisible->setChecked(true);
    printingSelectorDockWidget->setVisible(true);
}

void TabDeckEditor::loadLayout()
{
    LayoutsSettings &layouts = SettingsCache::instance().layouts();

    setCentralWidget(databaseDisplayDockWidget);

    auto &layoutState = layouts.getDeckEditorLayoutState();
    if (layoutState.isNull()) {
        restartLayout();
    } else {
        restoreState(layoutState);
        restoreGeometry(layouts.getDeckEditorGeometry());
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

    databaseDisplayDockWidget->setMinimumSize(100, 100);
    databaseDisplayDockWidget->setMaximumSize(1400, 5000);

    QTimer::singleShot(100, this, SLOT(freeDocksSize()));
}

void TabDeckEditor::restartLayout()
{
    deckDockWidget->setVisible(true);
    cardInfoDockWidget->setVisible(true);
    filterDockWidget->setVisible(true);
    printingSelectorDockWidget->setVisible(false);

    deckDockWidget->setFloating(false);
    cardInfoDockWidget->setFloating(false);
    filterDockWidget->setFloating(false);
    printingSelectorDockWidget->setFloating(false);

    aCardInfoDockVisible->setChecked(true);
    aDeckDockVisible->setChecked(true);
    aFilterDockVisible->setChecked(true);
    aPrintingSelectorDockVisible->setChecked(false);

    aCardInfoDockFloating->setChecked(false);
    aDeckDockFloating->setChecked(false);
    aFilterDockFloating->setChecked(false);
    aPrintingSelectorDockFloating->setChecked(false);

    setCentralWidget(databaseDisplayDockWidget);
    addDockWidget(static_cast<Qt::DockWidgetArea>(2), deckDockWidget);
    addDockWidget(static_cast<Qt::DockWidgetArea>(2), cardInfoDockWidget);
    addDockWidget(static_cast<Qt::DockWidgetArea>(2), filterDockWidget);
    addDockWidget(static_cast<Qt::DockWidgetArea>(2), printingSelectorDockWidget);

    splitDockWidget(cardInfoDockWidget, printingSelectorDockWidget, Qt::Horizontal);
    splitDockWidget(printingSelectorDockWidget, deckDockWidget, Qt::Horizontal);
    splitDockWidget(cardInfoDockWidget, printingSelectorDockWidget, Qt::Horizontal);
    splitDockWidget(cardInfoDockWidget, filterDockWidget, Qt::Vertical);

    QTimer::singleShot(100, this, SLOT(freeDocksSize()));
}

void TabDeckEditor::freeDocksSize()
{
    deckDockWidget->setMinimumSize(100, 100);
    deckDockWidget->setMaximumSize(5000, 5000);

    cardInfoDockWidget->setMinimumSize(100, 100);
    cardInfoDockWidget->setMaximumSize(5000, 5000);

    filterDockWidget->setMinimumSize(100, 100);
    filterDockWidget->setMaximumSize(5000, 5000);

    printingSelectorDockWidget->setMinimumSize(525, 100);
    printingSelectorDockWidget->setMaximumSize(5000, 5000);

    databaseDisplayDockWidget->setMinimumSize(100, 100);
    databaseDisplayDockWidget->setMaximumSize(1400, 5000);
}

void TabDeckEditor::dockVisibleTriggered()
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

void TabDeckEditor::dockFloatingTriggered()
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

void TabDeckEditor::dockTopLevelChanged(bool topLevel)
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

// Method uses to sync docks state with menu items state
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