#include "card_info_picture_widget.h"

#include "../../../../game/cards/card_item.h"
#include "../../picture_loader.h"

#include <QMouseEvent>
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

    const QSize scaledSize = resizedPixmap.size().scaled(size(), Qt::KeepAspectRatio);
    const QPoint topLeft{(width() - scaledSize.width()) / 2, (height() - scaledSize.height()) / 2};
    const qreal radius = 0.05 * scaledSize.width();

    QStylePainter painter(this);
    QPainterPath shape;
    shape.addRoundedRect(QRect(topLeft, scaledSize), radius, radius);
    painter.setClipPath(shape);
    painter.drawItemPixmap(QRect(topLeft, scaledSize), Qt::AlignCenter, resizedPixmap);
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
        const QPointF cursorPos = QCursor::pos();
        enlargedPixmapWidget->move(QPoint(static_cast<int>(cursorPos.x()) + enlargedPixmapOffset,
                                          static_cast<int>(cursorPos.y()) + enlargedPixmapOffset));
    }
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

    const QPointF cursorPos = QCursor::pos();
    enlargedPixmapWidget->move(static_cast<int>(cursorPos.x()) + enlargedPixmapOffset,
                               static_cast<int>(cursorPos.y()) + enlargedPixmapOffset);

    enlargedPixmapWidget->show();
}
