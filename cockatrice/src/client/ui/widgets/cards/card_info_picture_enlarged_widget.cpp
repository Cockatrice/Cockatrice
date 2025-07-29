#include "card_info_picture_enlarged_widget.h"

#include "../../../../settings/cache_settings.h"
#include "../../picture_loader/picture_loader.h"

#include <QPainterPath>
#include <QStylePainter>
#include <utility>

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
    if (card) {
        PictureLoader::getPixmap(enlargedPixmap, card, size);
    } else {
        PictureLoader::getCardBackPixmap(enlargedPixmap, size);
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

    // Scale the size of the pixmap to fit the widget while maintaining the aspect ratio
    QSize scaledSize = enlargedPixmap.size().scaled(size().width(), size().height(), Qt::KeepAspectRatio);

    // Calculate the position to center the scaled pixmap
    QPoint topLeft{(width() - scaledSize.width()) / 2, (height() - scaledSize.height()) / 2};

    // Define the radius for rounded corners
    // Adjust the radius as needed for rounded corners
    qreal radius = SettingsCache::instance().getRoundCardCorners() ? 0.05 * scaledSize.width() : 0.;

    QStylePainter painter(this);
    // Fill the background with transparent color to ensure rounded corners are rendered properly
    painter.fillRect(rect(), Qt::transparent); // Use the transparent background

    QPainterPath shape;
    shape.addRoundedRect(QRect(topLeft, scaledSize), radius, radius);
    painter.setClipPath(shape); // Set the clipping path

    // Draw the pixmap scaled to the calculated size
    painter.drawItemPixmap(QRect(topLeft, scaledSize), Qt::AlignCenter,
                           enlargedPixmap.scaled(scaledSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}
