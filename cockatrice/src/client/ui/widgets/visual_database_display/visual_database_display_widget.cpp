#include "visual_database_display_widget.h"

#include "../../../../deck/custom_line_edit.h"
#include "../../../../game/cards/card_database.h"
#include "../../../../game/cards/card_database_manager.h"
#include "../../../../game/filters/filter_tree_model.h"
#include "../../../../settings/cache_settings.h"
#include "../../../../utility/card_info_comparator.h"
#include "../cards/card_info_picture_with_text_overlay_widget.h"
#include "../quick_settings/settings_button_widget.h"
#include "visual_database_display_color_filter_widget.h"
#include "visual_database_display_filter_save_load_widget.h"
#include "visual_database_display_main_type_filter_widget.h"
#include "visual_database_display_name_filter_widget.h"
#include "visual_database_display_set_filter_widget.h"
#include "visual_database_display_sub_type_filter_widget.h"

#include <QHeaderView>
#include <QScrollBar>
#include <qpropertyanimation.h>

VisualDatabaseDisplayWidget::VisualDatabaseDisplayWidget(QWidget *parent,
                                                         AbstractTabDeckEditor *_deckEditor,
                                                         CardDatabaseModel *database_model,
                                                         CardDatabaseDisplayModel *database_display_model)
    : QWidget(parent), deckEditor(_deckEditor), databaseModel(database_model),
      databaseDisplayModel(database_display_model)
{
    cards = new QList<CardInfoPtr>;
    connect(databaseDisplayModel, &CardDatabaseDisplayModel::modelDirty, this,
            &VisualDatabaseDisplayWidget::modelDirty);
    connect(databaseDisplayModel, &QSortFilterProxyModel::invalidate, this, &VisualDatabaseDisplayWidget::modelDirty);

    // Set up main layout and widgets
    setMinimumSize(0, 0);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    main_layout = new QVBoxLayout(this);
    setLayout(main_layout);

    auto quickFilterWidget = new SettingsButtonWidget(this);

    auto searchLayout = new QHBoxLayout(this);

    searchEdit = new SearchLineEdit();
    searchEdit->setObjectName("searchEdit");
    searchEdit->setPlaceholderText(tr("Search by card name (or search expressions)"));
    searchEdit->setClearButtonEnabled(true);
    // searchEdit->addAction(loadColorAdjustedPixmap("theme:icons/search"), QLineEdit::LeadingPosition);
    // auto help = searchEdit->addAction(QPixmap("theme:icons/info"), QLineEdit::TrailingPosition);
    searchEdit->installEventFilter(&searchKeySignals);

    setFocusProxy(searchEdit);
    setFocusPolicy(Qt::ClickFocus);

    filterModel = new FilterTreeModel();
    filterModel->setObjectName("filterModel");
    databaseDisplayModel->setFilterTree(filterModel->filterTree());

    auto saveLoadWidget =
        new VisualDatabaseDisplayFilterSaveLoadWidget(this, filterModel, "/home/ascor/Cockatrice_presets/");
    quickFilterWidget->addSettingsWidget(saveLoadWidget);

    auto nameFilterWidget = new VisualDatabaseDisplayNameFilterWidget(this, deckEditor, filterModel);
    quickFilterWidget->addSettingsWidget(nameFilterWidget);

    auto mainTypeFilterWidget = new VisualDatabaseDisplayMainTypeFilterWidget(this, filterModel);
    quickFilterWidget->addSettingsWidget(mainTypeFilterWidget);

    auto subTypeFilterWidget = new VisualDatabaseDisplaySubTypeFilterWidget(this, filterModel);
    quickFilterWidget->addSettingsWidget(subTypeFilterWidget);

    auto setFilterWidget = new VisualDatabaseDisplaySetFilterWidget(this, filterModel);
    quickFilterWidget->addSettingsWidget(setFilterWidget);

    colorFilterWidget = new VisualDatabaseDisplayColorFilterWidget(this, filterModel);

    connect(filterModel, &FilterTreeModel::layoutChanged, this, &VisualDatabaseDisplayWidget::searchModelChanged);

    searchKeySignals.setObjectName("searchKeySignals");
    connect(searchEdit, SIGNAL(textChanged(const QString &)), this, SLOT(updateSearch(const QString &)));
    /*connect(&searchKeySignals, SIGNAL(onEnter()), this, SLOT(actAddCard()));
    connect(&searchKeySignals, SIGNAL(onCtrlAltEqual()), this, SLOT(actAddCard()));
    connect(&searchKeySignals, SIGNAL(onCtrlAltRBracket()), this, SLOT(actAddCardToSideboard()));
    connect(&searchKeySignals, SIGNAL(onCtrlAltMinus()), this, SLOT(actDecrementCard()));
    connect(&searchKeySignals, SIGNAL(onCtrlAltLBracket()), this, SLOT(actDecrementCardFromSideboard()));
    connect(&searchKeySignals, SIGNAL(onCtrlAltEnter()), this, SLOT(actAddCardToSideboard()));
    connect(&searchKeySignals, SIGNAL(onCtrlEnter()), this, SLOT(actAddCardToSideboard()));
    connect(&searchKeySignals, SIGNAL(onCtrlC()), this, SLOT(copyDatabaseCellContents()));*/
    // connect(help, &QAction::triggered, this, &DeckEditorDatabaseDisplayWidget::showSearchSyntaxHelp);

    /*databaseModel = new CardDatabaseModel(CardDatabaseManager::getInstance(), true, this);
    databaseModel->setObjectName("databaseModel");
    databaseDisplayModel = new CardDatabaseDisplayModel(this);
    databaseDisplayModel->setSourceModel(databaseModel);
    databaseDisplayModel->setFilterKeyColumn(0);*/

    databaseView = new QTreeView(this);
    databaseView->setObjectName("databaseView");
    databaseView->setFocusProxy(searchEdit);
    databaseView->setRootIsDecorated(false);
    databaseView->setItemDelegate(nullptr);
    databaseView->setSortingEnabled(true);
    databaseView->sortByColumn(0, Qt::AscendingOrder);
    databaseView->setModel(databaseDisplayModel);
    databaseView->setVisible(false);

    /*connect(databaseView->selectionModel(), SIGNAL(currentRowChanged(const QModelIndex &, const QModelIndex &)),
            deckEditor, SLOT(updateCardInfoLeft(const QModelIndex &, const QModelIndex &)));
    connect(databaseView->selectionModel(), SIGNAL(currentRowChanged(const QModelIndex &, const QModelIndex &)),
            deckEditor, SLOT(updatePrintingSelectorDatabase(const QModelIndex &, const QModelIndex &)));*/

    searchEdit->setTreeView(databaseView);

    searchLayout->addWidget(colorFilterWidget);
    searchLayout->addWidget(quickFilterWidget);
    searchLayout->addWidget(searchEdit);

    main_layout->addLayout(searchLayout);

    flow_widget = new FlowWidget(this, Qt::Horizontal, Qt::ScrollBarAlwaysOff, Qt::ScrollBarPolicy::ScrollBarAsNeeded);
    main_layout->addWidget(flow_widget);

    cardSizeWidget = new CardSizeWidget(this, flow_widget);
    main_layout->addWidget(cardSizeWidget);

    debounce_timer = new QTimer(this);
    debounce_timer->setSingleShot(true); // Ensure it only fires once after the timeout

    connect(debounce_timer, &QTimer::timeout, this, &VisualDatabaseDisplayWidget::searchModelChanged);

    auto loadCardsTimer = new QTimer(this);
    loadCardsTimer->setSingleShot(true); // Ensure it only fires once after the timeout

    connect(loadCardsTimer, &QTimer::timeout, this, [this]() { loadCurrentPage(); });
    loadCardsTimer->start(5000);
}

void VisualDatabaseDisplayWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    loadCurrentPage();
}

void VisualDatabaseDisplayWidget::onClick(QMouseEvent *event, CardInfoPictureWithTextOverlayWidget *instance)
{
    emit cardClickedDatabaseDisplay(event, instance);
}

void VisualDatabaseDisplayWidget::onHover(CardInfoPtr hoveredCard)
{
    emit cardHoveredDatabaseDisplay(hoveredCard);
}

void VisualDatabaseDisplayWidget::addCard(CardInfoPtr cardToAdd)
{
    cards->append(cardToAdd);
    CardInfoPictureWithTextOverlayWidget *display = new CardInfoPictureWithTextOverlayWidget(flow_widget, false);
    display->setScaleFactor(cardSizeWidget->getSlider()->value());
    display->setCard(cardToAdd);
    flow_widget->addWidget(display);
    connect(display, SIGNAL(imageClicked(QMouseEvent *, CardInfoPictureWithTextOverlayWidget *)), this,
            SLOT(onClick(QMouseEvent *, CardInfoPictureWithTextOverlayWidget *)));
    connect(display, SIGNAL(hoveredOnCard(CardInfoPtr)), this, SLOT(onHover(CardInfoPtr)));
    connect(cardSizeWidget->getSlider(), &QSlider::valueChanged, display, &CardInfoPictureWidget::setScaleFactor);
}

void VisualDatabaseDisplayWidget::populateCards()
{
    int rowCount = databaseDisplayModel->rowCount();
    cards->clear();

    // Calculate the start and end indices for the current page
    int start = currentPage * cardsPerPage;
    int end = qMin(start + cardsPerPage, rowCount);

    qCDebug(VisualDatabaseDisplayLog) << "Fetching from " << start << " to " << end << " cards";
    // Load more cards if we are at the end of the current list and can fetch more
    if (end >= rowCount && databaseDisplayModel->canFetchMore(QModelIndex())) {
        qCDebug(VisualDatabaseDisplayLog) << "We gotta load more";
        databaseDisplayModel->fetchMore(QModelIndex());
    }

    for (int row = start; row < end; ++row) {
        qCDebug(VisualDatabaseDisplayLog) << "Adding " << row;
        QModelIndex index = databaseDisplayModel->index(row, CardDatabaseModel::NameColumn);
        QVariant name = databaseDisplayModel->data(index, Qt::DisplayRole);
        qCDebug(VisualDatabaseDisplayLog) << name.toString();
        CardInfoPtr info = CardDatabaseManager::getInstance()->getCard(name.toString());
        if (info) {
            addCard(info);
        } else {
            qCDebug(VisualDatabaseDisplayLog) << "Card not found in database!";
        }
    }
    currentPage++;
}

void VisualDatabaseDisplayWidget::updateSearch(const QString &search)
{
    databaseDisplayModel->setStringFilter(search);
    QModelIndexList sel = databaseView->selectionModel()->selectedRows();
    if (sel.isEmpty() && databaseDisplayModel->rowCount())
        databaseView->selectionModel()->setCurrentIndex(databaseDisplayModel->index(0, 0),
                                                        QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
}

void VisualDatabaseDisplayWidget::searchModelChanged()
{
    // Clear the current page and prepare for new data
    flow_widget->clearLayout(); // Clear existing cards
    cards->clear();             // Clear the card list
    // Reset scrollbar position to the top after loading new cards
    QScrollBar *scrollBar = flow_widget->scrollArea->verticalScrollBar();
    if (scrollBar) {
        scrollBar->setValue(0); // Reset scrollbar to top
    }

    currentPage = 0;
    loadCurrentPage();
    qCDebug(VisualDatabaseDisplayLog) << "Search model changed";
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
    for (int row = start; row < end; ++row) {
        QModelIndex index = databaseDisplayModel->index(row, CardDatabaseModel::NameColumn);
        QVariant name = databaseDisplayModel->data(index, Qt::DisplayRole);
        CardInfoPtr info = CardDatabaseManager::getInstance()->getCard(name.toString());
        if (info) {
            addCard(info);
        } else {
            qCDebug(VisualDatabaseDisplayLog) << "Card not found in database!";
        }
    }

    // Update the current page
    currentPage++;
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

void VisualDatabaseDisplayWidget::modelDirty()
{
    debounce_timer->start(debounce_time);
}

void VisualDatabaseDisplayWidget::sortCardList(const QStringList properties, Qt::SortOrder order = Qt::AscendingOrder)
{
    CardInfoComparator comparator(properties, order);
    std::sort(cards->begin(), cards->end(), comparator);
}

void VisualDatabaseDisplayWidget::databaseDataChanged(QModelIndex topLeft, QModelIndex bottomRight)
{
    (void)topLeft;
    (void)bottomRight;
    qCDebug(VisualDatabaseDisplayLog) << "Database Data changed";
}

void VisualDatabaseDisplayWidget::wheelEvent(QWheelEvent *event)
{
    int totalRows = databaseDisplayModel->rowCount(); // Total number of cards
    int nextPageStartIndex = (currentPage + 1) * cardsPerPage;

    // Handle scrolling down
    if (event->angleDelta().y() < 0) {
        // Check if the next page has any cards to load
        if (nextPageStartIndex < totalRows) {
            loadCurrentPage(); // Load the next page
            event->accept();   // Accept the event as valid
            return;
        }
        qCDebug(VisualDatabaseDisplayLog) << nextPageStartIndex << ":" << totalRows;
    }

    // Prevent overscrolling when there's no more data to load
    event->ignore();
}
