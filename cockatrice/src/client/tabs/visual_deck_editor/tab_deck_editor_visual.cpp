#include "tab_deck_editor_visual.h"

#include "../../../deck/deck_list_model.h"
#include "../../../deck/deck_stats_interface.h"
#include "../../../dialogs/dlg_load_deck.h"
#include "../../../dialogs/dlg_load_deck_from_clipboard.h"
#include "../../../game/cards/card_database_model.h"
#include "../../../game/filters/filter_builder.h"
#include "../../../server/pending_command.h"
#include "../../../settings/cache_settings.h"
#include "../../ui/pixel_map_generator.h"
#include "../../ui/widgets/cards/card_info_frame_widget.h"
#include "../../ui/widgets/deck_analytics/deck_analytics_widget.h"
#include "../../ui/widgets/visual_deck_editor/visual_deck_editor_widget.h"
#include "../tab_deck_editor.h"
#include "../tab_supervisor.h"
#include "pb/command_deck_upload.pb.h"
#include "tab_deck_editor_visual_tab_widget.h"
#include "trice_limits.h"

#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QDir>
#include <QDockWidget>
#include <QFileDialog>
#include <QHeaderView>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QPrintPreviewDialog>
#include <QProcessEnvironment>
#include <QSplitter>
#include <QTextStream>
#include <QTimer>
#include <QTreeView>
#include <QVBoxLayout>

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
}

void TabDeckEditorVisual::createCentralFrame()
{
    centralWidget = new QWidget(this);
    centralWidget->setObjectName("centralWidget");

    centralFrame = new QVBoxLayout;
    centralWidget->setLayout(centralFrame);

    tabContainer = new TabDeckEditorVisualTabWidget(centralWidget, this, this->deckDockWidget->deckModel,
                                                    this->databaseDisplayDockWidget->databaseModel,
                                                    this->databaseDisplayDockWidget->databaseDisplayModel);
    connect(tabContainer, &TabDeckEditorVisualTabWidget::cardChanged, this,
            &TabDeckEditorVisual::changeModelIndexAndCardInfo);
    connect(tabContainer, &TabDeckEditorVisualTabWidget::cardChangedDatabaseDisplay, this,
            &AbstractTabDeckEditor::updateCard);
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
    updateCard(activeCard);
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
    if (event->button() == Qt::LeftButton) {
        actSwapCard(instance->getInfo(), DECK_ZONE_MAIN);
    } else if (event->button() == Qt::RightButton) {
        actDecrementCard(instance->getInfo());
    } else if (event->button() == Qt::MiddleButton) {
        deckDockWidget->actRemoveCard();
    }
}

void TabDeckEditorVisual::processSideboardCardClick(QMouseEvent *event, CardInfoPictureWithTextOverlayWidget *instance)
{
    if (event->button() == Qt::LeftButton) {
        actSwapCard(instance->getInfo(), DECK_ZONE_SIDE);
    } else if (event->button() == Qt::RightButton) {
        actDecrementCardFromSideboard(instance->getInfo());
    } else if (event->button() == Qt::MiddleButton) {
        deckDockWidget->actRemoveCard();
    }
}

void TabDeckEditorVisual::processCardClickDatabaseDisplay(QMouseEvent *event,
                                                          CardInfoPictureWithTextOverlayWidget *instance)
{
    if (event->button() == Qt::LeftButton) {
        actAddCard(instance->getInfo());
    } else if (event->button() == Qt::RightButton) {
        actDecrementCard(instance->getInfo());
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

    databaseDisplayDockWidget->setMinimumSize(100, 100);
    databaseDisplayDockWidget->setMaximumSize(1400, 5000);
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

    databaseDisplayDockWidget->setMinimumSize(100, 100);
    databaseDisplayDockWidget->setMaximumSize(1400, 5000);

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

void TabDeckEditorVisual::actLoadDeckFromClipboard()
{
    AbstractTabDeckEditor::actLoadDeckFromClipboard();
    tabContainer->visualDeckView->updateDisplay();
    tabContainer->deckAnalytics->refreshDisplays(deckDockWidget->deckModel);
    tabContainer->sampleHandWidget->setDeckModel(deckDockWidget->deckModel);
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
            TabDeckEditorVisual::setDeck(l);
        }
    } else {
        delete l;
        QMessageBox::critical(this, tr("Error"), tr("Could not open deck at %1").arg(fileName));
    }
    deckMenu->setSaveStatus(true);
}

void TabDeckEditorVisual::setDeck(DeckLoader *_deck)
{
    AbstractTabDeckEditor::setDeck(_deck);
    tabContainer->visualDeckView->updateDisplay();
    tabContainer->deckAnalytics->refreshDisplays(deckDockWidget->deckModel);
    tabContainer->sampleHandWidget->setDeckModel(deckDockWidget->deckModel);
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