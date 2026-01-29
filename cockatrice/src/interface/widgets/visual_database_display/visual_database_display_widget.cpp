#include "visual_database_display_widget.h"

#include "../../../client/settings/cache_settings.h"
#include "../../../filters/filter_tree_model.h"
#include "../../../filters/syntax_help.h"
#include "../../pixel_map_generator.h"
#include "../cards/card_info_picture_with_text_overlay_widget.h"
#include "../deck_editor/deck_state_manager.h"
#include "../general/tutorial/tutorial_controller.h"
#include "../quick_settings/settings_button_widget.h"
#include "../tabs/visual_deck_editor/tab_deck_editor_visual.h"
#include "../utility/custom_line_edit.h"
#include "visual_database_display_color_filter_widget.h"
#include "visual_database_display_filter_save_load_widget.h"
#include "visual_database_display_set_filter_widget.h"

#include <QHeaderView>
#include <QScrollBar>
#include <QStyledItemDelegate>
#include <libcockatrice/card/card_info_comparator.h>
#include <libcockatrice/card/database/card_database.h>
#include <libcockatrice/card/database/card_database_manager.h>
#include <libcockatrice/utility/qt_utils.h>

VisualDatabaseDisplayWidget::VisualDatabaseDisplayWidget(QWidget *parent,
                                                         AbstractTabDeckEditor *_deckEditor,
                                                         CardDatabaseModel *database_model,
                                                         CardDatabaseDisplayModel *database_display_model)
    : QWidget(parent), deckEditor(_deckEditor), databaseModel(database_model),
      databaseDisplayModel(database_display_model)
{
    cards = new QList<ExactCard>;
    connect(databaseDisplayModel, &CardDatabaseDisplayModel::modelDirty, this,
            &VisualDatabaseDisplayWidget::modelDirty);

    // Set up main layout and widgets
    setMinimumSize(0, 0);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mainLayout = new QVBoxLayout(this);
    setLayout(mainLayout);
    mainLayout->setSpacing(1);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    flowWidget = new FlowWidget(this, Qt::Horizontal, Qt::ScrollBarAlwaysOff, Qt::ScrollBarPolicy::ScrollBarAsNeeded);
    cardSizeWidget = new CardSizeWidget(this, flowWidget, SettingsCache::instance().getVisualDatabaseDisplayCardSize());
    connect(cardSizeWidget, &CardSizeWidget::cardSizeSettingUpdated, &SettingsCache::instance(),
            &SettingsCache::setVisualDatabaseDisplayCardSize);

    searchContainer = new QWidget(this);
    searchLayout = new QHBoxLayout(searchContainer);
    searchContainer->setLayout(searchLayout);

    searchEdit = new SearchLineEdit();
    searchEdit->setObjectName("searchEdit");
    searchEdit->setPlaceholderText(tr("Search by card name (or search expressions)"));
    searchEdit->setClearButtonEnabled(true);
    searchEdit->addAction(loadColorAdjustedPixmap("theme:icons/search"), QLineEdit::LeadingPosition);
    auto help = searchEdit->addAction(QPixmap("theme:icons/info"), QLineEdit::TrailingPosition);
    connect(help, &QAction::triggered, this, [this] { createSearchSyntaxHelpWindow(searchEdit); });
    searchEdit->installEventFilter(&searchKeySignals);

    setFocusProxy(searchEdit);
    setFocusPolicy(Qt::ClickFocus);

    displayModeButton = new QPushButton(tr("Visual"), this);
    displayModeButton->setCheckable(true); // Toggle button

    connect(displayModeButton, &QPushButton::toggled, this, &VisualDatabaseDisplayWidget::onDisplayModeChanged);

    displayModeButton->setChecked(false); // Start in Visual mode

    filterModel = new FilterTreeModel();
    filterModel->setObjectName("filterModel");

    searchKeySignals.setObjectName("searchKeySignals");
    connect(searchEdit, &SearchLineEdit::textChanged, this, &VisualDatabaseDisplayWidget::updateSearch);

    DeckEditorDatabaseDisplayWidget *databaseDisplayWidget = deckEditor->cardDatabaseDockWidget->databaseDisplayWidget;
    connect(&searchKeySignals, &KeySignals::onEnter, databaseDisplayWidget,
            &DeckEditorDatabaseDisplayWidget::actAddCardToMainDeck);
    connect(&searchKeySignals, &KeySignals::onCtrlAltEqual, databaseDisplayWidget,
            &DeckEditorDatabaseDisplayWidget::actAddCardToMainDeck);
    connect(&searchKeySignals, &KeySignals::onCtrlAltRBracket, databaseDisplayWidget,
            &DeckEditorDatabaseDisplayWidget::actAddCardToSideboard);
    connect(&searchKeySignals, &KeySignals::onCtrlAltMinus, databaseDisplayWidget,
            &DeckEditorDatabaseDisplayWidget::actDecrementCardFromMainDeck);
    connect(&searchKeySignals, &KeySignals::onCtrlAltLBracket, databaseDisplayWidget,
            &DeckEditorDatabaseDisplayWidget::actDecrementCardFromSideboard);
    connect(&searchKeySignals, &KeySignals::onCtrlAltEnter, databaseDisplayWidget,
            &DeckEditorDatabaseDisplayWidget::actAddCardToSideboard);
    connect(&searchKeySignals, &KeySignals::onCtrlEnter, databaseDisplayWidget,
            &DeckEditorDatabaseDisplayWidget::actAddCardToSideboard);
    connect(&searchKeySignals, &KeySignals::onCtrlC, databaseDisplayWidget,
            &DeckEditorDatabaseDisplayWidget::copyDatabaseCellContents);
    connect(help, &QAction::triggered, this, [this] { createSearchSyntaxHelpWindow(searchEdit); });

    databaseView = databaseDisplayWidget->getDatabaseView();
    databaseView->setFocusProxy(searchEdit);
    databaseView->setItemDelegate(nullptr);
    databaseView->setVisible(false);

    searchEdit->setTreeView(databaseView);

    colorFilterWidget = new VisualDatabaseDisplayColorFilterWidget(this, filterModel);

    filterContainer = new VisualDatabaseDisplayFilterToolbarWidget(this);

    clearFilterWidget = new QToolButton();
    clearFilterWidget->setFixedSize(32, 32);
    clearFilterWidget->setIcon(QPixmap("theme:icons/delete"));
    connect(clearFilterWidget, &QToolButton::clicked, this, [this] {
        filterModel->blockSignals(true);
        filterModel->filterTree()->blockSignals(true);
        filterModel->clear();
        filterModel->blockSignals(false);
        filterModel->filterTree()->blockSignals(false);
        emit filterModel->filterTree()->changed();
        emit filterModel->layoutChanged();
    });

    databaseLoadIndicator = new QLabel(this);
    databaseLoadIndicator->setAlignment(Qt::AlignCenter);

    mainLayout->addWidget(databaseLoadIndicator);

    if (CardDatabaseManager::getInstance()->getLoadStatus() != LoadStatus::Ok) {
        connect(CardDatabaseManager::getInstance(), &CardDatabase::cardDatabaseLoadingFinished, this,
                &VisualDatabaseDisplayWidget::initialize);
        filterContainer->setVisible(false);
    } else {
        initialize();
        databaseLoadIndicator->setVisible(false);
    }

    retranslateUi();
}

TutorialSequence VisualDatabaseDisplayWidget::addTutorialSteps()
{
    auto sequence = TutorialSequence();
    sequence.addStep({colorFilterWidget, "Filter the database by colors with these controls"});
    TutorialStep displayModeStep;
    displayModeStep.targetWidget = displayModeButton;
    displayModeStep.text = tr("You can change back to the old table display-style with this button.");
    displayModeStep.allowClickThrough = true;
    sequence.addStep(displayModeStep);
    sequence.addStep({filterContainer, "Use these controls for quick access to common filters."});

    TutorialStep setFilterStep;
    setFilterStep.targetWidget = filterContainer->getSetFilterWidget();
    setFilterStep.text = tr("Let's try it out now by selecting a set filter!");
    setFilterStep.allowClickThrough = true;
    setFilterStep.requiresInteraction = true;
    setFilterStep.autoAdvanceOnValid = true;
    setFilterStep.validationTiming = ValidationTiming::OnSignal;
    setFilterStep.signalSource = filterModel;
    setFilterStep.signalName = SIGNAL(layoutChanged());
    setFilterStep.validator = [] { return true; };
    sequence.addStep(setFilterStep);

    TutorialStep explorationStep;
    explorationStep.targetWidget = this;
    explorationStep.text = tr(
        "Try it out!\n\nWe've cleared the previous deck. Add 5 different new cards to the deck by clicking on them!");
    explorationStep.allowClickThrough = true;
    explorationStep.requiresInteraction = true;
    explorationStep.autoAdvanceOnValid = true;
    explorationStep.validationTiming = ValidationTiming::OnSignal;
    if (QtUtils::findParentOfType<TabDeckEditorVisual>(this)) {
        explorationStep.onEnter = [this] {
            QtUtils::findParentOfType<TabDeckEditorVisual>(this)->deckStateManager->clearDeck();
        };
        explorationStep.signalSource =
            QtUtils::findParentOfType<TabDeckEditorVisual>(this)->deckStateManager->getModel();
        explorationStep.signalName = SIGNAL(cardNodesChanged());
        explorationStep.validator = [this] {
            if (QtUtils::findParentOfType<TabDeckEditorVisual>(this)) {
                return QtUtils::findParentOfType<TabDeckEditorVisual>(this)
                           ->deckStateManager->getModel()
                           ->getDeckList()
                           ->getCardList()
                           .size() >= 5;
            }
            return true;
        };
    }

    sequence.addStep(explorationStep);

    TutorialStep conclusionStep;
    conclusionStep.targetWidget = this;
    conclusionStep.text = tr(
        "Great!\n\nLet's look at them in the deck view before we conclude this tutorial with the analytics widgets.");
    conclusionStep.onExit = [this]() {
        auto tabWidget = QtUtils::findParentOfType<TabDeckEditorVisualTabWidget>(this);
        if (tabWidget) {
            tabWidget->setCurrentWidget(tabWidget->visualDeckView);
        }
    };

    sequence.addStep(conclusionStep);

    /*sequence.addStep(
        {quickFilterSaveLoadWidget, "This button will let you save and load all currently applied filters to files."});
    sequence.addStep({quickFilterNameWidget,
                      "This button will let you apply name filters. Optionally, you can import every card in "
                      "your deck as a name filter and then save this as a filter using the save/load button "
                      "to make your own quick access collections!"});
    sequence.addStep({mainTypeFilterWidget, "Use these buttons to quickly filter by card types."});
    sequence.addStep({quickFilterSubTypeWidget, "This button will let you apply filters for card sub-types."});
    sequence.addStep(
        {quickFilterSetWidget,
         "This button will let you apply filters for card sets. You can also filter to the X most recent sets. "
         "Filtering to a set will display all printings of a card within that set."});*/

    return sequence;
}

void VisualDatabaseDisplayWidget::initialize()
{
    databaseLoadIndicator->setVisible(false);

    filterContainer->initialize();

    searchLayout->addWidget(colorFilterWidget);
    searchLayout->addWidget(clearFilterWidget);
    searchLayout->addWidget(searchEdit);
    searchLayout->addWidget(displayModeButton);

    mainLayout->addWidget(searchContainer);

    mainLayout->addWidget(filterContainer);

    mainLayout->addWidget(databaseView);

    mainLayout->addWidget(flowWidget);

    mainLayout->addWidget(cardSizeWidget);

    debounceTimer = new QTimer(this);
    debounceTimer->setSingleShot(true); // Ensure it only fires once after the timeout

    connect(debounceTimer, &QTimer::timeout, this, &VisualDatabaseDisplayWidget::onSearchModelChanged);

    databaseDisplayModel->setFilterTree(filterModel->filterTree());

    connect(filterModel, &FilterTreeModel::layoutChanged, this, &VisualDatabaseDisplayWidget::onSearchModelChanged);

    loadCardsTimer = new QTimer(this);
    loadCardsTimer->setSingleShot(true); // Ensure it only fires once after the timeout

    connect(loadCardsTimer, &QTimer::timeout, this, [this]() { loadCurrentPage(); });
    loadCardsTimer->start(5000);

    retranslateUi();
}

void VisualDatabaseDisplayWidget::retranslateUi()
{
    databaseLoadIndicator->setText(tr("Loading database ..."));
    clearFilterWidget->setToolTip(tr("Clear all filters"));
}

void VisualDatabaseDisplayWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    loadCurrentPage();
}

void VisualDatabaseDisplayWidget::onDisplayModeChanged(bool checked)
{
    if (checked) {
        // Table mode
        displayModeButton->setText(tr("Table"));
        flowWidget->setVisible(false);
        cardSizeWidget->setVisible(false);
        databaseView->setItemDelegate(new QStyledItemDelegate(databaseView));
        databaseView->setVisible(true);
    } else {
        // Visual mode
        displayModeButton->setText(tr("Visual"));
        flowWidget->setVisible(true);
        cardSizeWidget->setVisible(true);
        databaseView->setVisible(false);
        populateCards();
    }
}

void VisualDatabaseDisplayWidget::onClick(QMouseEvent *event, CardInfoPictureWithTextOverlayWidget *instance)
{
    emit cardClickedDatabaseDisplay(event, instance);
}

void VisualDatabaseDisplayWidget::onHover(const ExactCard &hoveredCard)
{
    emit cardHoveredDatabaseDisplay(hoveredCard);
}

void VisualDatabaseDisplayWidget::addCard(const ExactCard &cardToAdd)
{
    cards->append(cardToAdd);
    auto *display = new CardInfoPictureWithTextOverlayWidget(flowWidget, false);
    display->setScaleFactor(cardSizeWidget->getSlider()->value());
    display->setCard(cardToAdd);
    flowWidget->addWidget(display);
    connect(display, &CardInfoPictureWithTextOverlayWidget::imageClicked, this, &VisualDatabaseDisplayWidget::onClick);
    connect(display, &CardInfoPictureWithTextOverlayWidget::hoveredOnCard, this, &VisualDatabaseDisplayWidget::onHover);
    connect(cardSizeWidget->getSlider(), &QSlider::valueChanged, display, &CardInfoPictureWidget::setScaleFactor);
}

void VisualDatabaseDisplayWidget::updateSearch(const QString &search) const
{
    databaseDisplayModel->setStringFilter(search);
    QModelIndexList sel = databaseView->selectionModel()->selectedRows();
    if (sel.isEmpty() && databaseDisplayModel->rowCount())
        databaseView->selectionModel()->setCurrentIndex(databaseDisplayModel->index(0, 0),
                                                        QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
}

void VisualDatabaseDisplayWidget::onSearchModelChanged()
{
    if (flowWidget->isVisible()) {
        // Clear the current page and prepare for new data
        flowWidget->clearLayout(); // Clear existing cards
        cards->clear();            // Clear the card list
        // Reset scrollbar position to the top after loading new cards
        if (QScrollBar *scrollBar = flowWidget->scrollArea->verticalScrollBar()) {
            scrollBar->setValue(0); // Reset scrollbar to top
        }

        currentPage = 0;
        loadCurrentPage();
        qCDebug(VisualDatabaseDisplayLog) << "Search model changed";
    }
}

void VisualDatabaseDisplayWidget::loadCurrentPage()
{
    // Ensure only the initial page is loaded
    if (currentPage == 0) {
        // Only load the first page initially
        qCDebug(VisualDatabaseDisplayLog) << "Loading the first page";
        populateCards();
    } else {
        // If not the first page, just load the next page and append to the flow widget
        loadNextPage();
    }
}

void VisualDatabaseDisplayWidget::populateCards()
{
    cards->clear();
    loadNextPage();
}

void VisualDatabaseDisplayWidget::loadNextPage()
{
    // Calculate the start and end indices for the next page
    int rowCount = databaseDisplayModel->rowCount();
    int start = currentPage * cardsPerPage;
    int end = qMin(start + cardsPerPage, rowCount);

    // Load more cards if we are at the end of the current list and can fetch more
    if (end >= rowCount && databaseDisplayModel->canFetchMore(QModelIndex())) {
        databaseDisplayModel->fetchMore(QModelIndex());
    }

    // Load the next page of cards and add them to the flow widget
    loadPage(start, end);
}

void VisualDatabaseDisplayWidget::loadPage(int start, int end)
{
    QList<const CardFilter *> setFilters = filterModel->getFiltersOfType(CardFilter::AttrSet);
    for (int row = start; row < end; ++row) {
        qCDebug(VisualDatabaseDisplayLog) << "Adding " << row;
        QModelIndex index = databaseDisplayModel->index(row, CardDatabaseModel::NameColumn);
        QVariant name = databaseDisplayModel->data(index, Qt::DisplayRole);
        qCDebug(VisualDatabaseDisplayLog) << name.toString();

        if (CardInfoPtr info = CardDatabaseManager::query()->getCardInfo(name.toString())) {
            if (!setFilters.empty()) {
                SetToPrintingsMap setMap = info->getSets();
                for (const CardFilter *setFilter : setFilters) {
                    if (setMap.contains(setFilter->term())) {
                        for (PrintingInfo printing : setMap[setFilter->term()]) {
                            addCard(ExactCard(info, printing));
                        }
                    }
                }
            } else {
                addCard(CardDatabaseManager::query()->getPreferredCard(info));
            }
        } else {
            qCDebug(VisualDatabaseDisplayLog) << "Card not found in database!";
        }
    }
    currentPage++;
}

void VisualDatabaseDisplayWidget::modelDirty() const
{
    debounceTimer->start(debounceTime);
}

void VisualDatabaseDisplayWidget::sortCardList(const QStringList &properties,
                                               Qt::SortOrder order = Qt::AscendingOrder) const
{
    CardInfoComparator comparator(properties, order);
    std::sort(cards->begin(), cards->end(), [comparator](const ExactCard &a, const ExactCard &b) {
        return comparator(a.getCardPtr(), b.getCardPtr());
    });
}

void VisualDatabaseDisplayWidget::databaseDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    (void)topLeft;
    (void)bottomRight;
    qCDebug(VisualDatabaseDisplayLog) << "Database Data changed";
}

void VisualDatabaseDisplayWidget::wheelEvent(QWheelEvent *event)
{
    int totalCards = databaseDisplayModel->rowCount(); // Total number of cards
    int loadedCards = currentPage * cardsPerPage;

    // Handle scrolling down
    if (event->angleDelta().y() < 0) {
        // Check if the next page has any cards to load
        if (loadedCards < totalCards) {
            loadCurrentPage(); // Load the next page
            event->accept();   // Accept the event as valid
            return;
        }
        qCDebug(VisualDatabaseDisplayLog) << loadedCards << ":" << totalCards;
    }

    // Prevent overscrolling when there's no more data to load
    event->ignore();
}
