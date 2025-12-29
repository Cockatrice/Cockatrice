#include "card_info_picture_enlarged_widget.h"

#include "../../../client/settings/cache_settings.h"
#include "../../../interface/card_picture_loader/card_picture_loader.h"

#include <QPainterPath>
#include <QStylePainter>

/**
 * @brief Constructs a CardPictureEnlargedWidget.
 * @param parent The parent widget.
 *
 * Sets the widget's window flags to keep it displayed as a tooltip overlay.
 */
CardInfoPictureEnlargedWidget::CardInfoPictureEnlargedWidget(QWidget *parent) : QWidget(parent), pixmapDirty(true)
{
    setWindowFlags(Qt::ToolTip); // Keeps this widget on top of everything
    setAttribute(Qt::WA_TranslucentBackground);

    connect(&SettingsCache::instance(), &SettingsCache::roundCardCornersChanged, this, [this](bool _roundCardCorners) {
        Q_UNUSED(_roundCardCorners);

        update();
    });
}

/**
 * @brief Loads the pixmap based on the given size and card information.
 * @param size The desired size for the loaded pixmap.
 *
 * If card information is available, it loads the card's specific pixmap. Otherwise, it loads a default card back
 * pixmap.
 */
void CardInfoPictureEnlargedWidget::loadPixmap(const QSize &size)
{
    // Handle DPI scaling
    qreal dpr = devicePixelRatio();   // Get the actual scaling factor
    QSize availableSize = size * dpr; // Convert to physical pixel size
    if (card) {
        CardPictureLoader::getPixmap(enlargedPixmap, card, availableSize);
    } else {
        CardPictureLoader::getCardBackPixmap(enlargedPixmap, availableSize);
    }
    pixmapDirty = false;
}

/**
 * @brief Sets the pixmap for the widget based on a provided card.
 * @param _card The card information to load.
 * @param size The desired size for the pixmap.
 *
 * Sets the widget's pixmap to the card image and resizes the widget to match the specified size. Triggers a repaint.
 */
void CardInfoPictureEnlargedWidget::setCardPixmap(const ExactCard &_card, const QSize size)
{
    card = _card;
    loadPixmap(size);

    setFixedSize(size); // Set the widget size to the enlarged size

    update(); // Trigger a repaint
}

/**
 * @brief Custom paint event that draws the enlarged card image with rounded corners.
 * @param event The paint event (unused).
 *
 * Checks if the pixmap is valid. Then, calculates the size and position for centering the
 * scaled pixmap within the widget, applies rounded corners, and draws the pixmap.
 */
void CardInfoPictureEnlargedWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    if (width() == 0 || height() == 0 || enlargedPixmap.isNull()) {
        return;
    }

    if (pixmapDirty) {
        loadPixmap(size());
    }

    qreal dpr = enlargedPixmap.devicePixelRatio();

    QSize logicalPixmapSize(enlargedPixmap.width() / dpr, enlargedPixmap.height() / dpr);

    // Scale the pixmap to fit the widget (logical → logical)
    QSize scaledLogicalSize = logicalPixmapSize.scaled(size(), Qt::KeepAspectRatio);

    // Convert scaled logical size → physical size for scaled()
    QSize scaledPhysicalSize = scaledLogicalSize * dpr;

    // Pixmap scaled in PHYSICAL pixels
    QPixmap finalPixmap = enlargedPixmap.scaled(scaledPhysicalSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    finalPixmap.setDevicePixelRatio(dpr);

    // Center inside widget
    QPoint topLeft{(width() - scaledLogicalSize.width()) / 2, (height() - scaledLogicalSize.height()) / 2};

    // Rounded corner radius based on logical width
    qreal radius = SettingsCache::instance().getRoundCardCorners() ? 0.05 * scaledLogicalSize.width() : 0.0;

    QStylePainter painter(this);
    // Fill the background with transparent color to ensure rounded corners are rendered properly
    painter.fillRect(rect(), Qt::transparent); // Use the transparent background

    QPainterPath shape;
    shape.addRoundedRect(QRect(topLeft, scaledLogicalSize), radius, radius);
    painter.setClipPath(shape); // Set the clipping path

    // Draw the pixmap scaled to the calculated size
    painter.drawPixmap(QRect(topLeft, scaledLogicalSize), finalPixmap);
}
