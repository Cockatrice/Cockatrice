#include "visual_database_display_widget.h"

#include "../../../client/settings/cache_settings.h"
#include "../../../filters/filter_tree_model.h"
#include "../../../filters/syntax_help.h"
#include "../../pixel_map_generator.h"
#include "../cards/card_info_picture_with_text_overlay_widget.h"
#include "../quick_settings/settings_button_widget.h"
#include "../utility/custom_line_edit.h"
#include "visual_database_display_color_filter_widget.h"
#include "visual_database_display_filter_save_load_widget.h"
#include "visual_database_display_main_type_filter_widget.h"
#include "visual_database_display_name_filter_widget.h"
#include "visual_database_display_set_filter_widget.h"
#include "visual_database_display_sub_type_filter_widget.h"

#include <QHeaderView>
#include <QScrollBar>
#include <QStyledItemDelegate>
#include <libcockatrice/card/card_info_comparator.h>
#include <libcockatrice/card/database/card_database.h>
#include <libcockatrice/card/database/card_database_manager.h>
#include <utility>

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
    connect(&searchKeySignals, &KeySignals::onEnter, deckEditor->databaseDisplayDockWidget,
            &DeckEditorDatabaseDisplayWidget::actAddCardToMainDeck);
    connect(&searchKeySignals, &KeySignals::onCtrlAltEqual, deckEditor->databaseDisplayDockWidget,
            &DeckEditorDatabaseDisplayWidget::actAddCardToMainDeck);
    connect(&searchKeySignals, &KeySignals::onCtrlAltRBracket, deckEditor->databaseDisplayDockWidget,
            &DeckEditorDatabaseDisplayWidget::actAddCardToSideboard);
    connect(&searchKeySignals, &KeySignals::onCtrlAltMinus, deckEditor->databaseDisplayDockWidget,
            &DeckEditorDatabaseDisplayWidget::actDecrementCardFromMainDeck);
    connect(&searchKeySignals, &KeySignals::onCtrlAltLBracket, deckEditor->databaseDisplayDockWidget,
            &DeckEditorDatabaseDisplayWidget::actDecrementCardFromSideboard);
    connect(&searchKeySignals, &KeySignals::onCtrlAltEnter, deckEditor->databaseDisplayDockWidget,
            &DeckEditorDatabaseDisplayWidget::actAddCardToSideboard);
    connect(&searchKeySignals, &KeySignals::onCtrlEnter, deckEditor->databaseDisplayDockWidget,
            &DeckEditorDatabaseDisplayWidget::actAddCardToSideboard);
    connect(&searchKeySignals, &KeySignals::onCtrlC, deckEditor->databaseDisplayDockWidget,
            &DeckEditorDatabaseDisplayWidget::copyDatabaseCellContents);
    connect(help, &QAction::triggered, this, [this] { createSearchSyntaxHelpWindow(searchEdit); });

    databaseView = deckEditor->databaseDisplayDockWidget->getDatabaseView();
    databaseView->setFocusProxy(searchEdit);
    databaseView->setItemDelegate(nullptr);
    databaseView->setVisible(false);

    searchEdit->setTreeView(databaseView);

    sortByLabel = new QLabel(this);
    sortColumnCombo = new QComboBox(this);
    sortColumnCombo->setSizeAdjustPolicy(QComboBox::SizeAdjustPolicy::AdjustToContents);
    sortOrderCombo = new QComboBox(this);
    sortOrderCombo->setSizeAdjustPolicy(QComboBox::SizeAdjustPolicy::AdjustToContents);

    sortOrderCombo->addItem("Ascending", Qt::AscendingOrder);
    sortOrderCombo->addItem("Descending", Qt::DescendingOrder);
    sortOrderCombo->view()->setMinimumWidth(sortOrderCombo->view()->sizeHintForColumn(0));
    sortOrderCombo->adjustSize();

    // Populate columns dynamically from the model
    for (int i = 0; i < databaseDisplayModel->columnCount(); ++i) {
        QString header = databaseDisplayModel->headerData(i, Qt::Horizontal).toString();
        sortColumnCombo->addItem(header, i);
    }

    sortColumnCombo->view()->setMinimumWidth(sortColumnCombo->view()->sizeHintForColumn(0));
    sortColumnCombo->adjustSize();

    connect(sortColumnCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() {
        int column = sortColumnCombo->currentData().toInt();
        Qt::SortOrder order = static_cast<Qt::SortOrder>(sortOrderCombo->currentData().toInt());
        databaseView->sortByColumn(column, order);

        searchModelChanged();
    });

    connect(sortOrderCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() {
        int column = sortColumnCombo->currentData().toInt();
        Qt::SortOrder order = static_cast<Qt::SortOrder>(sortOrderCombo->currentData().toInt());
        databaseView->sortByColumn(column, order);

        searchModelChanged();
    });

    colorFilterWidget = new VisualDatabaseDisplayColorFilterWidget(this, filterModel);

    filterContainer = new QWidget(this);
    filterContainerLayout = new QHBoxLayout(filterContainer);
    filterContainer->setLayout(filterContainerLayout);

    filterByLabel = new QLabel(this);

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

    quickFilterSaveLoadWidget = new SettingsButtonWidget(this);
    quickFilterSaveLoadWidget->setButtonIcon(QPixmap("theme:icons/lock"));

    quickFilterNameWidget = new SettingsButtonWidget(this);
    quickFilterNameWidget->setButtonIcon(QPixmap("theme:icons/rename"));

    quickFilterSubTypeWidget = new SettingsButtonWidget(this);
    quickFilterSubTypeWidget->setButtonIcon(QPixmap("theme:icons/player"));

    quickFilterSetWidget = new SettingsButtonWidget(this);
    quickFilterSetWidget->setButtonIcon(QPixmap("theme:icons/scales"));

    filterContainer->setMaximumHeight(80);

    databaseLoadIndicator = new QLabel(this);
    databaseLoadIndicator->setAlignment(Qt::AlignCenter);

    mainLayout->addWidget(databaseLoadIndicator);

    if (CardDatabaseManager::getInstance()->getLoadStatus() != LoadStatus::Ok) {
        connect(CardDatabaseManager::getInstance(), &CardDatabase::cardDatabaseLoadingFinished, this,
                &VisualDatabaseDisplayWidget::initialize);
        sortByLabel->setVisible(false);
        filterByLabel->setVisible(false);
        quickFilterSaveLoadWidget->setVisible(false);
        quickFilterNameWidget->setVisible(false);
        quickFilterSubTypeWidget->setVisible(false);
        quickFilterSetWidget->setVisible(false);
    } else {
        initialize();
        databaseLoadIndicator->setVisible(false);
    }

    retranslateUi();
}

void VisualDatabaseDisplayWidget::initialize()
{
    databaseLoadIndicator->setVisible(false);

    sortByLabel->setVisible(true);
    filterByLabel->setVisible(true);
    quickFilterSaveLoadWidget->setVisible(true);
    quickFilterNameWidget->setVisible(true);
    quickFilterSubTypeWidget->setVisible(true);
    quickFilterSetWidget->setVisible(true);

    saveLoadWidget = new VisualDatabaseDisplayFilterSaveLoadWidget(this, filterModel);
    nameFilterWidget = new VisualDatabaseDisplayNameFilterWidget(this, deckEditor, filterModel);
    mainTypeFilterWidget = new VisualDatabaseDisplayMainTypeFilterWidget(this, filterModel);
    subTypeFilterWidget = new VisualDatabaseDisplaySubTypeFilterWidget(this, filterModel);
    setFilterWidget = new VisualDatabaseDisplaySetFilterWidget(this, filterModel);

    quickFilterSaveLoadWidget->addSettingsWidget(saveLoadWidget);
    quickFilterNameWidget->addSettingsWidget(nameFilterWidget);
    quickFilterSubTypeWidget->addSettingsWidget(subTypeFilterWidget);
    quickFilterSetWidget->addSettingsWidget(setFilterWidget);

    filterContainerLayout->addWidget(sortByLabel);
    filterContainerLayout->addWidget(sortColumnCombo);
    filterContainerLayout->addWidget(sortOrderCombo);
    filterContainerLayout->addWidget(filterByLabel);
    filterContainerLayout->addWidget(quickFilterSaveLoadWidget);
    filterContainerLayout->addWidget(quickFilterNameWidget);
    filterContainerLayout->addWidget(quickFilterSubTypeWidget);
    filterContainerLayout->addWidget(quickFilterSetWidget);
    filterContainerLayout->addWidget(mainTypeFilterWidget);

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

    connect(debounceTimer, &QTimer::timeout, this, &VisualDatabaseDisplayWidget::searchModelChanged);

    databaseDisplayModel->setFilterTree(filterModel->filterTree());

    connect(filterModel, &FilterTreeModel::layoutChanged, this, &VisualDatabaseDisplayWidget::searchModelChanged);

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

    sortByLabel->setText(tr("Sort by:"));
    filterByLabel->setText(tr("Filter by:"));

    quickFilterSaveLoadWidget->setToolTip(tr("Save and load filters"));
    quickFilterNameWidget->setToolTip(tr("Filter by exact card name"));
    quickFilterSubTypeWidget->setToolTip(tr("Filter by card sub-type"));
    quickFilterSetWidget->setToolTip(tr("Filter by set"));
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

void VisualDatabaseDisplayWidget::searchModelChanged()
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
