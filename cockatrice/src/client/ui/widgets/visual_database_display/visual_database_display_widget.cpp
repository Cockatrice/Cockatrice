#include "visual_database_display_widget.h"

#include "../../../../game/cards/card_database.h"
#include "../../../../game/cards/card_database_manager.h"
#include "../../../../main.h"
#include "../../../../utility/card_info_comparator.h"
#include "../cards/card_info_picture_with_text_overlay_widget.h"

#include <QPushButton>
#include <qpropertyanimation.h>

VisualDatabaseDisplayWidget::VisualDatabaseDisplayWidget(QWidget *parent,
                                                         CardDatabaseModel *database_model,
                                                         CardDatabaseDisplayModel *database_display_model)
    : QWidget(parent), databaseModel(database_model), databaseDisplayModel(database_display_model)
{
    cards = new QList<CardInfoPtr>;
    connect(databaseDisplayModel, &CardDatabaseDisplayModel::modelDirty, this,
            &VisualDatabaseDisplayWidget::modelDirty);

    // Set up main layout and widgets
    setMinimumSize(0, 0);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    main_layout = new QVBoxLayout();
    setLayout(main_layout);

    flow_widget = new FlowWidget(this, Qt::ScrollBarAlwaysOff, Qt::ScrollBarPolicy::ScrollBarAsNeeded);
    main_layout->addWidget(flow_widget);

    overlap_control_widget = new OverlapControlWidget(80, 10, 10, Qt::Vertical, flow_widget);
    main_layout->addWidget(overlap_control_widget);

    debounce_timer = new QTimer(this);
    debounce_timer->setSingleShot(true); // Ensure it only fires once after the timeout

    connect(debounce_timer, &QTimer::timeout, this, &VisualDatabaseDisplayWidget::searchModelChanged);
    setupPaginationControls();
    loadCurrentPage(); // Load the first page of cards
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

void VisualDatabaseDisplayWidget::populateCards()
{
    this->cards->clear();
    int rowCount = databaseDisplayModel->rowCount();

    // Calculate the start and end indices for the current page
    int start = currentPage * cardsPerPage;
    int end = qMin(start + cardsPerPage, rowCount);

    // Load more cards if we are at the end of the current list and can fetch more
    if (end >= rowCount && databaseDisplayModel->canFetchMore(QModelIndex())) {
        databaseDisplayModel->fetchMore(QModelIndex());
    }

    for (int row = start; row < end; ++row) {
        QModelIndex index = databaseDisplayModel->index(row, CardDatabaseModel::NameColumn);
        QVariant name = databaseDisplayModel->data(index, Qt::DisplayRole);
        CardInfoPtr info = CardDatabaseManager::getInstance()->getCard(name.toString());
        cards->append(info);
    }
}

void VisualDatabaseDisplayWidget::searchModelChanged()
{
    currentPage = 0;
    loadCurrentPage();
}

void VisualDatabaseDisplayWidget::loadCurrentPage()
{
    // If an animation is already in progress, don't start a new one
    if (isAnimating) {
        return;
    }

    // Set the flag indicating animation is in progress
    isAnimating = true;

    // Store the original position of the widget
    QPoint originalPos = flow_widget->pos();

    // Create the first animation to slide the flow_widget
    QPropertyAnimation *slideOutAnimation = new QPropertyAnimation(flow_widget, "pos");
    slideOutAnimation->setDuration(300);           // Adjust for smoothness
    slideOutAnimation->setStartValue(originalPos); // Start from the current position

    // Adjust the position (e.g., slide it up or down)
    QPoint slideOutPos = originalPos;
    slideOutPos.setY(originalPos.y() - flow_widget->height()); // Slide up (adjust direction as needed)
    slideOutAnimation->setEndValue(slideOutPos);               // End at the new position

    // Create the second animation to bring it back to its original position
    QPropertyAnimation *slideBackAnimation = new QPropertyAnimation(flow_widget, "pos");
    slideBackAnimation->setDuration(300);           // Duration for smooth transition
    slideBackAnimation->setStartValue(slideOutPos); // Start from where the first animation ended
    slideBackAnimation->setEndValue(originalPos);   // End at the original position

    // Connect the end of the first animation to the start of the second
    connect(slideOutAnimation, &QPropertyAnimation::finished, [this, slideBackAnimation]() {
        // After sliding out, adjust the number of cards, load the new page, and start the slide back animation
        adjustCardsPerPage(); // Adjust the cards per page before loading
        populateCards();      // Load the new page of cards
        updateDisplay();      // Update the visual display

        // Start sliding back to original position
        slideBackAnimation->start(QAbstractAnimation::DeleteWhenStopped);
    });

    // Once the second animation finishes, allow new animations to start
    connect(slideBackAnimation, &QPropertyAnimation::finished, [this]() {
        isAnimating = false; // Reset the flag after animation finishes
    });

    // Start the slide-out animation
    slideOutAnimation->start(QAbstractAnimation::DeleteWhenStopped);
}

void VisualDatabaseDisplayWidget::updateDisplay()
{
    // Clear the layout first
    flow_widget->clearLayout();

    OverlapWidget *printings_group_widget = new OverlapWidget(flow_widget, 0, cardsPerRow, rowsPerColumn, Qt::Vertical);

    // Create card widgets and store their sizes
    QList<CardInfoPictureWithTextOverlayWidget *> cardDisplays;
    for (const auto &info : *cards) {
        if (info) {
            CardInfoPictureWithTextOverlayWidget *display =
                new CardInfoPictureWithTextOverlayWidget(printings_group_widget, true);
            display->setCard(info);
            cardDisplays.append(display);
            printings_group_widget->addWidget(display);
            connect(display, SIGNAL(imageClicked(QMouseEvent *, CardInfoPictureWithTextOverlayWidget *)), this,
                    SLOT(onClick(QMouseEvent *, CardInfoPictureWithTextOverlayWidget *)));
            connect(display, SIGNAL(hoveredOnCard(CardInfoPtr)), this, SLOT(onHover(CardInfoPtr)));
        } else {
            qDebug() << "Card not found in database!";
        }
    }

    // Calculate the maximum size of the card widgets after they've been added to the layout
    // adjustCardsPerPage();

    overlap_control_widget->connectOverlapWidget(printings_group_widget);
    flow_widget->addWidget(printings_group_widget);
    flow_widget->update();

    update(); // Update the widget display
}

void VisualDatabaseDisplayWidget::adjustCardsPerPage()
{
    // Calculate available width and height in the OverlapWidget
    int availableWidth = flow_widget->width();
    int availableHeight = flow_widget->height();

    QList<CardInfoPictureWithTextOverlayWidget *> cardDisplays;
    for (OverlapWidget *child : flow_widget->findChildren<OverlapWidget *>()) {
        for (CardInfoPictureWithTextOverlayWidget *overlapChild :
             child->findChildren<CardInfoPictureWithTextOverlayWidget *>()) {
            if (CardInfoPictureWithTextOverlayWidget *cardDisplay =
                    qobject_cast<CardInfoPictureWithTextOverlayWidget *>(overlapChild)) {
                cardDisplays.append(cardDisplay);
            }
        }
    }

    // Measure the size of the first card display to determine the size of cards
    if (!cardDisplays.isEmpty()) {
        int cardWidth = cardDisplays.first()->sizeHint().width();
        int cardHeight = cardDisplays.first()->sizeHint().height();
        const int overlapMargin = 10; // Margin between cards

        // Calculate how many cards can fit horizontally and vertically
        cardsPerRow = availableWidth / (cardWidth + overlapMargin);
        rowsPerColumn = availableHeight / (cardHeight + overlapMargin);
        // qDebug() << "available width " << availableWidth << "available height: " << availableHeight;
        // qDebug() << "width: " << cardWidth << "height: " << cardHeight << "cardsPerRow: " << cardsPerRow <<
        // "rowsPerColumn: " << rowsPerColumn;

        // Update cardsPerPage based on rows and columns
        cardsPerPage = cardsPerRow * rowsPerColumn;
        // qDebug() << "Adjusted cards per page to:" << cardsPerPage;
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
    updateDisplay();
}

void VisualDatabaseDisplayWidget::wheelEvent(QWheelEvent *event)
{
    if (isAnimating) {
        return;
    }
    // Check scroll direction
    if (event->angleDelta().y() > 0) {
        // Scrolling up
        if (currentPage > 0) {
            currentPage--;
            loadCurrentPage(); // Load the previous page
        }
    } else if (event->angleDelta().y() < 0) {
        // Scrolling down
        if ((currentPage + 1) * cardsPerPage < databaseDisplayModel->rowCount()) {
            currentPage++;
            loadCurrentPage(); // Load the next page
        }
    }

    // Prevent overscrolling by stopping the event if needed
    if ((currentPage == 0 && event->angleDelta().y() > 0) ||
        ((currentPage + 1) * cardsPerPage >= databaseDisplayModel->rowCount() && event->angleDelta().y() < 0)) {
        event->ignore(); // Ignore the event to prevent overscrolling
    } else {
        event->accept(); // Accept the event if scrolling is valid
    }
}

void VisualDatabaseDisplayWidget::setupPaginationControls()
{
    QPushButton *prevButton = new QPushButton("Previous", this);
    QPushButton *nextButton = new QPushButton("Next", this);

    connect(prevButton, &QPushButton::clicked, this, [this]() {
        if (currentPage > 0) {
            currentPage--;
            loadCurrentPage();
        }
    });

    connect(nextButton, &QPushButton::clicked, this, [this]() {
        if ((currentPage + 1) * cardsPerPage < databaseDisplayModel->rowCount()) {
            currentPage++;
            loadCurrentPage();
        }
    });

    // Add buttons to layout (for example, at the bottom of the main layout)
    main_layout->addWidget(prevButton);
    main_layout->addWidget(nextButton);
}
