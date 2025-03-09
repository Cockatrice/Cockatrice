#include "card_info_picture_widget.h"

#include "../../../../game/cards/card_database_manager.h"
#include "../../../../game/cards/card_item.h"
#include "../../../../settings/cache_settings.h"
#include "../../../tabs/tab_deck_editor.h"
#include "../../../tabs/tab_supervisor.h"
#include "../../picture_loader/picture_loader.h"
#include "../../window_main.h"

#include <QMenu>
#include <QMouseEvent>
#include <QScreen>
#include <QStylePainter>
#include <QWidget>
#include <utility>

/**
 * @class CardInfoPictureWidget
 * @brief Widget that displays an enlarged image of a card, loading the image based on the card's info or showing a
 * default image.
 *
 * This widget can optionally display a larger version of the card's image when hovered over,
 * depending on the `hoverToZoomEnabled` parameter.
 */

/**
 * @brief Constructs a CardInfoPictureWidget.
 * @param parent The parent widget, if any.
 * @param hoverToZoomEnabled If this widget will spawn a larger widget when hovered over.
 *
 * Initializes the widget with a minimum height and sets the pixmap to a dirty state for initial loading.
 */
CardInfoPictureWidget::CardInfoPictureWidget(QWidget *parent, const bool hoverToZoomEnabled)
    : QWidget(parent), info(nullptr), pixmapDirty(true), hoverToZoomEnabled(hoverToZoomEnabled)
{
    setMinimumHeight(baseHeight);
    if (hoverToZoomEnabled) {
        setMouseTracking(true);
    }

    enlargedPixmapWidget = new CardInfoPictureEnlargedWidget(this);
    enlargedPixmapWidget->hide();

    hoverTimer = new QTimer(this);
    hoverTimer->setSingleShot(true);
    connect(hoverTimer, &QTimer::timeout, this, &CardInfoPictureWidget::showEnlargedPixmap);
}

/**
 * @brief Sets the card to be displayed and updates the pixmap.
 * @param card A shared pointer to the card information (CardInfoPtr).
 *
 * Disconnects any existing signal connections from the previous card info and connects to the `pixmapUpdated`
 * signal of the new card to automatically update the pixmap when the card image changes.
 */
void CardInfoPictureWidget::setCard(CardInfoPtr card)
{
    if (info) {
        disconnect(info.data(), nullptr, this, nullptr);
    }

    info = std::move(card);

    if (info) {
        connect(info.data(), SIGNAL(pixmapUpdated()), this, SLOT(updatePixmap()));
    }

    updatePixmap();
}

/**
 * @brief Sets the hover to zoom feature.
 * @param enabled If true, enables the hover-to-zoom functionality; otherwise, disables it.
 */
void CardInfoPictureWidget::setHoverToZoomEnabled(const bool enabled)
{
    hoverToZoomEnabled = enabled;
    setMouseTracking(enabled);
}

/**
 * @brief Handles widget resizing by updating the pixmap size.
 * @param event The resize event (unused).
 *
 * Calls `updatePixmap()` to ensure the image scales appropriately when the widget is resized.
 */
void CardInfoPictureWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updatePixmap();
}

/**
 * @brief Sets the scale factor for the widget.
 * @param scale The scale factor to apply.
 *
 * Adjusts the widget's size according to the scale factor and updates the pixmap.
 */
void CardInfoPictureWidget::setScaleFactor(const int scale)
{
    const int newWidth = baseWidth * scale / 100;
    const int newHeight = static_cast<int>(newWidth * aspectRatio);

    scaleFactor = scale;

    setFixedSize(newWidth, newHeight);
    updatePixmap();

    emit cardScaleFactorChanged(scale);
}

/**
 * @brief Marks the pixmap as dirty and triggers a widget repaint.
 *
 * Sets `pixmapDirty` to true, indicating that the pixmap needs to be reloaded before the next display.
 */
void CardInfoPictureWidget::updatePixmap()
{
    pixmapDirty = true;
    update();
}

/**
 * @brief Loads the appropriate pixmap based on the current card info.
 *
 * If `info` is valid, loads the card's image. Otherwise, loads a default card back image.
 */
void CardInfoPictureWidget::loadPixmap()
{
    PictureLoader::getCardBackLoadingInProgressPixmap(resizedPixmap, size());
    if (info) {
        PictureLoader::getPixmap(resizedPixmap, info, size());
    } else {
        PictureLoader::getCardBackLoadingFailedPixmap(resizedPixmap, size());
    }

    pixmapDirty = false;
}

/**
 * @brief Custom paint event that draws the card image with rounded corners.
 * @param event The paint event (unused).
 *
 * Checks if the pixmap needs to be reloaded. Then, calculates the size and position for centering the
 * scaled pixmap within the widget, applies rounded corners, and draws the pixmap.
 */
void CardInfoPictureWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    if (width() == 0 || height() == 0) {
        return;
    }

    if (pixmapDirty) {
        loadPixmap();
    }

    QPixmap transformedPixmap = resizedPixmap; // Default pixmap
    if (SettingsCache::instance().getAutoRotateSidewaysLayoutCards()) {
        if (info && info->getLandscapeOrientation()) {
            // Rotate pixmap 90 degrees to the left
            QTransform transform;
            transform.rotate(90);
            transformedPixmap = resizedPixmap.transformed(transform, Qt::SmoothTransformation);
        }
    }

    // Handle DPI scaling
    qreal dpr = devicePixelRatio();     // Get the actual scaling factor
    QSize availableSize = size() * dpr; // Convert to physical pixel size

    // Compute final scaled size
    QSize pixmapSize = transformedPixmap.size();
    QSize scaledSize = pixmapSize.scaled(availableSize, Qt::KeepAspectRatio);

    // Pre-scale the pixmap once before drawing
    QPixmap finalPixmap = transformedPixmap.scaled(scaledSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    finalPixmap.setDevicePixelRatio(dpr); // Ensure correct display on high-DPI screens

    // Compute target rectangle with explicit integer conversion
    int targetX = static_cast<int>((availableSize.width() - scaledSize.width()) / (2 * dpr));
    int targetY = static_cast<int>((availableSize.height() - scaledSize.height()) / (2 * dpr));
    int targetW = static_cast<int>(scaledSize.width() / dpr);
    int targetH = static_cast<int>(scaledSize.height() / dpr);
    QRect targetRect{targetX, targetY, targetW, targetH};

    // Compute rounded corner radius
    qreal radius = 0.05 * static_cast<qreal>(targetRect.width()); // Ensure consistent rounding

    // Draw the pixmap with rounded corners
    QStylePainter painter(this);
    QPainterPath shape;
    shape.addRoundedRect(targetRect, radius, radius);
    painter.setClipPath(shape);

    // Draw the pre-scaled pixmap directly
    painter.drawPixmap(targetRect, finalPixmap);
}

/**
 * @brief Provides the recommended size for the widget based on the scale factor.
 * @return The recommended widget size.
 */
QSize CardInfoPictureWidget::sizeHint() const
{
    return {static_cast<int>(baseWidth * scaleFactor), static_cast<int>(baseHeight * scaleFactor)};
}

/**
 * @brief Starts the hover timer to show the enlarged pixmap on hover.
 * @param event The enter event.
 */
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void CardInfoPictureWidget::enterEvent(QEnterEvent *event)
#else
void CardInfoPictureWidget::enterEvent(QEvent *event)
#endif
{
    QWidget::enterEvent(event); // Call the base class implementation

    // If hover-to-zoom is enabled, start the hover timer
    if (hoverToZoomEnabled) {
        hoverTimer->start(hoverActivateThresholdInMs);
    }

    // Emit signal indicating a card is being hovered on
    emit hoveredOnCard(info);
}

/**
 * @brief Stops the hover timer and hides the enlarged pixmap when the mouse leaves.
 * @param event The leave event.
 */
void CardInfoPictureWidget::leaveEvent(QEvent *event)
{
    QWidget::leaveEvent(event);
    if (hoverToZoomEnabled) {
        hoverTimer->stop();
        enlargedPixmapWidget->hide();
    }
}

/**
 * @brief Moves the enlarged pixmap widget to follow the mouse cursor.
 * @param event The mouse move event.
 */
void CardInfoPictureWidget::mouseMoveEvent(QMouseEvent *event)
{
    QWidget::mouseMoveEvent(event);

    if (hoverToZoomEnabled && enlargedPixmapWidget->isVisible()) {
        const QPoint cursorPos = QCursor::pos();
        const QRect screenGeometry = QGuiApplication::screenAt(cursorPos)->geometry();
        const QSize widgetSize = enlargedPixmapWidget->size();

        int newX = cursorPos.x() + enlargedPixmapOffset;
        int newY = cursorPos.y() + enlargedPixmapOffset;

        // Adjust if out of bounds
        if (newX + widgetSize.width() > screenGeometry.right()) {
            newX = cursorPos.x() - widgetSize.width() - enlargedPixmapOffset;
        }
        if (newY + widgetSize.height() > screenGeometry.bottom()) {
            newY = cursorPos.y() - widgetSize.height() - enlargedPixmapOffset;
        }

        enlargedPixmapWidget->move(newX, newY);
    }
}

void CardInfoPictureWidget::mousePressEvent(QMouseEvent *event)
{
    QWidget::mousePressEvent(event);
    if (event->button() == Qt::RightButton) {
        createRightClickMenu()->popup(QCursor::pos());
    }
}

QMenu *CardInfoPictureWidget::createRightClickMenu()
{
    auto *cardMenu = new QMenu(this);

    if (!info) {
        return cardMenu;
    }

    cardMenu->addMenu(createViewRelatedCardsMenu());
    cardMenu->addMenu(createAddToOpenDeckMenu());

    return cardMenu;
}

QMenu *CardInfoPictureWidget::createViewRelatedCardsMenu()
{
    auto viewRelatedCards = new QMenu(tr("View related cards"));

    QList<CardRelation *> relatedCards = info->getAllRelatedCards();

    auto relatedCardExists = [](const CardRelation *cardRelation) {
        return CardDatabaseManager::getInstance()->getCard(cardRelation->getName()) != nullptr;
    };

    bool atLeastOneGoodRelationFound = std::any_of(relatedCards.begin(), relatedCards.end(), relatedCardExists);

    if (!atLeastOneGoodRelationFound) {
        viewRelatedCards->setEnabled(false);
        return viewRelatedCards;
    }

    for (const auto &relatedCard : relatedCards) {
        const auto &relatedCardName = relatedCard->getName();
        QAction *viewCard = viewRelatedCards->addAction(relatedCardName);
        connect(viewCard, &QAction::triggered, this, [this, &relatedCardName] {
            emit cardChanged(CardDatabaseManager::getInstance()->getCard(relatedCardName));
        });
        viewRelatedCards->addAction(viewCard);
    }

    return viewRelatedCards;
}

QMenu *CardInfoPictureWidget::createAddToOpenDeckMenu()
{
    auto addToOpenDeckMenu = new QMenu(tr("Add card to deck"));

    auto *mainWindow = qobject_cast<MainWindow *>(window());
    QList<AbstractTabDeckEditor *> deckEditorTabs = mainWindow->getTabSupervisor()->getDeckEditorTabs();

    if (deckEditorTabs.isEmpty()) {
        addToOpenDeckMenu->setEnabled(false);
        return addToOpenDeckMenu;
    }

    for (auto &deckEditorTab : deckEditorTabs) {
        auto *addCardMenu = addToOpenDeckMenu->addMenu(deckEditorTab->getTabText());

        QAction *addCard = addCardMenu->addAction(tr("Mainboard"));
        connect(addCard, &QAction::triggered, this, [this, deckEditorTab] {
            deckEditorTab->updateCard(info);
            deckEditorTab->actAddCard(info);
        });

        QAction *addCardSideboard = addCardMenu->addAction(tr("Sideboard"));
        connect(addCardSideboard, &QAction::triggered, this, [this, deckEditorTab] {
            deckEditorTab->updateCard(info);
            deckEditorTab->actAddCardToSideboard(info);
        });
    }

    return addToOpenDeckMenu;
}

/**
 * @brief Displays the enlarged version of the card's pixmap near the cursor.
 *
 * If card information is available, the enlarged pixmap is loaded, positioned near the cursor,
 * and displayed.
 */
void CardInfoPictureWidget::showEnlargedPixmap() const
{
    if (!info) {
        return;
    }

    const QSize enlargedSize(static_cast<int>(size().width() * scaleFactor),
                             static_cast<int>(size().width() * aspectRatio * scaleFactor));
    enlargedPixmapWidget->setCardPixmap(info, enlargedSize);

    const QPoint cursorPos = QCursor::pos();
    const QRect screenGeometry = QGuiApplication::screenAt(cursorPos)->geometry();
    const QSize widgetSize = enlargedPixmapWidget->size();

    int newX = cursorPos.x() + enlargedPixmapOffset;
    int newY = cursorPos.y() + enlargedPixmapOffset;

    // Adjust if out of bounds
    if (newX + widgetSize.width() > screenGeometry.right()) {
        newX = cursorPos.x() - widgetSize.width() - enlargedPixmapOffset;
    }
    if (newY + widgetSize.height() > screenGeometry.bottom()) {
        newY = cursorPos.y() - widgetSize.height() - enlargedPixmapOffset;
    }

    enlargedPixmapWidget->move(newX, newY);

    enlargedPixmapWidget->show();
}
