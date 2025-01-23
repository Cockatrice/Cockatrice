#include "visual_database_display_widget.h"

#include "../../../../game/cards/card_database.h"
#include "../../../../game/cards/card_database_manager.h"
#include "../../../../main.h"
#include "../../../../utility/card_info_comparator.h"
#include "../cards/card_info_picture_with_text_overlay_widget.h"

#include <QPushButton>
#include <QScrollBar>
#include <qpropertyanimation.h>

VisualDatabaseDisplayWidget::VisualDatabaseDisplayWidget(QWidget *parent,
                                                         CardDatabaseModel *database_model,
                                                         CardDatabaseDisplayModel *database_display_model)
    : QWidget(parent), databaseModel(database_model), databaseDisplayModel(database_display_model)
{
    cards = new QList<CardInfoPtr>;
    connect(databaseDisplayModel, &CardDatabaseDisplayModel::modelDirty, this,
            &VisualDatabaseDisplayWidget::modelDirty);
    connect(databaseDisplayModel, &QSortFilterProxyModel::invalidate, this, &VisualDatabaseDisplayWidget::modelDirty);

    // Set up main layout and widgets
    setMinimumSize(0, 0);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    main_layout = new QVBoxLayout();
    setLayout(main_layout);

    flow_widget = new FlowWidget(this, Qt::Horizontal, Qt::ScrollBarAlwaysOff, Qt::ScrollBarPolicy::ScrollBarAsNeeded);
    main_layout->addWidget(flow_widget);

    cardSizeWidget = new CardSizeWidget(this, flow_widget);
    main_layout->addWidget(cardSizeWidget);

    debounce_timer = new QTimer(this);
    debounce_timer->setSingleShot(true); // Ensure it only fires once after the timeout

    connect(debounce_timer, &QTimer::timeout, this, &VisualDatabaseDisplayWidget::searchModelChanged);

    auto loadCardsTimer = new QTimer(this);
    loadCardsTimer->setSingleShot(true); // Ensure it only fires once after the timeout

    connect(loadCardsTimer, &QTimer::timeout, this, [this]() {
        qDebug() << "timer timed out";
        loadCurrentPage();
    });
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

    qDebug() << "Fetching from " << start << " to " << end << " cards";
    // Load more cards if we are at the end of the current list and can fetch more
    if (end >= rowCount && databaseDisplayModel->canFetchMore(QModelIndex())) {
        qDebug() << "We gotta load more";
        databaseDisplayModel->fetchMore(QModelIndex());
    }

    for (int row = start; row < end; ++row) {
        qDebug() << "Adding " << row;
        QModelIndex index = databaseDisplayModel->index(row, CardDatabaseModel::NameColumn);
        QVariant name = databaseDisplayModel->data(index, Qt::DisplayRole);
        qDebug() << name.toString();
        CardInfoPtr info = CardDatabaseManager::getInstance()->getCard(name.toString());
        if (info) {
            addCard(info);
        } else {
            qDebug() << "Card not found in database!";
        }
    }
    currentPage++;
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
    qDebug() << "Search model changed";
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
            qDebug() << "Card not found in database!";
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
        qDebug() << "Loading the first page";
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
    qDebug() << "Database Data changed";
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
        qDebug() << nextPageStartIndex << ":" << totalRows;
    }

    // Prevent overscrolling when there's no more data to load
    event->ignore();
}
