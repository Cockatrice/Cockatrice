#include "tab_deck_editor_visual.h"

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
#include <libcockatrice/settings/cache_settings.h>
#include <libcockatrice/utility/trice_limits.h>

TabDeckEditorVisual::TabDeckEditorVisual(TabSupervisor *_tabSupervisor) : AbstractTabDeckEditor(_tabSupervisor)
{
    setObjectName("TabDeckEditorVisual");

    createCentralFrame();

    TabDeckEditorVisual::createMenus();

    installEventFilter(this);

    TabDeckEditorVisual::retranslateUi();
    connect(&SettingsCache::instance().shortcuts(), SIGNAL(shortCutChanged()), this, SLOT(refreshShortcuts()));
    TabDeckEditorVisual::refreshShortcuts();

    TabDeckEditorVisual::loadLayout();
    databaseDisplayDockWidget->setHidden(true);
}

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

void TabDeckEditorVisual::onDeckChanged()
{
    AbstractTabDeckEditor::onDeckModified();
    tabContainer->visualDeckView->constructZoneWidgetsFromDeckListModel();
    tabContainer->deckAnalytics->refreshDisplays(deckDockWidget->deckModel);
    tabContainer->sampleHandWidget->setDeckModel(deckDockWidget->deckModel);
}

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

QString TabDeckEditorVisual::getTabText() const
{
    QString result = tr("Visual Deck: %1").arg(deckDockWidget->getSimpleDeckName());
    if (modified)
        result.prepend("* ");
    return result;
}

void TabDeckEditorVisual::changeModelIndexAndCardInfo(const ExactCard &activeCard)
{
    updateCard(activeCard);
    changeModelIndexToCard(activeCard);
}

void TabDeckEditorVisual::changeModelIndexToCard(const ExactCard &activeCard)
{
    QString cardName = activeCard.getName();
    QModelIndex index = deckDockWidget->deckModel->findCard(cardName, DECK_ZONE_MAIN);
    if (!index.isValid()) {
        index = deckDockWidget->deckModel->findCard(cardName, DECK_ZONE_SIDE);
    }
    deckDockWidget->deckView->setCurrentIndex(index);
}

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

bool TabDeckEditorVisual::actSaveDeckAs()
{
    // We have to disable the quick-add search bar or else it'll steal focus after dialog creation.
    tabContainer->visualDeckView->searchBar->setEnabled(false);
    auto result = AbstractTabDeckEditor::actSaveDeckAs();
    tabContainer->visualDeckView->searchBar->setEnabled(true);
    return result;
}

void TabDeckEditorVisual::showPrintingSelector()
{
    printingSelectorDockWidget->printingSelector->setCard(cardInfoDockWidget->cardInfo->getCard().getCardPtr(),
                                                          DECK_ZONE_MAIN);
    printingSelectorDockWidget->printingSelector->updateDisplay();
    aPrintingSelectorDockVisible->setChecked(true);
    printingSelectorDockWidget->setVisible(true);
}

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