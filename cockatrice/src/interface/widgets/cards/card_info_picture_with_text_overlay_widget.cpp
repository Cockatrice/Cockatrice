#include "card_info_picture_with_text_overlay_widget.h"

#include <QFontMetrics>
#include <QPainterPath>
#include <QStylePainter>

/**
 * @brief Constructs a CardPictureWithTextOverlay widget.
 * @param parent The parent widget.
 * @param hoverToZoomEnabled If this widget will spawn a larger widget when hovered over.
 * @param raiseOnEnter If this widget will raise slightly when entered.
 * @param textColor The color of the overlay text.
 * @param outlineColor The color of the outline around the text.
 * @param fontSize The font size of the overlay text.
 * @param alignment The alignment of the text within the overlay.
 *
 * Sets the widget's size policy and default border style.
 */
CardInfoPictureWithTextOverlayWidget::CardInfoPictureWithTextOverlayWidget(QWidget *parent,
                                                                           const bool hoverToZoomEnabled,
                                                                           const bool raiseOnEnter,
                                                                           const QColor &textColor,
                                                                           const QColor &outlineColor,
                                                                           const int fontSize,
                                                                           const Qt::Alignment alignment)
    : CardInfoPictureWidget(parent, hoverToZoomEnabled, raiseOnEnter), textColor(textColor), outlineColor(outlineColor),
      fontSize(fontSize), textAlignment(alignment), highlighted(false)
{
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

/**
 * @brief Sets the overlay text to be displayed on the card.
 * @param text The text to overlay.
 *
 * Updates the widget to display the new overlay text.
 */
void CardInfoPictureWithTextOverlayWidget::setOverlayText(const QString &text)
{
    overlayText = text;
    update(); // Trigger a redraw to display the updated text
}

/**
 * @brief Sets the color of the overlay text.
 * @param color The new text color.
 */
void CardInfoPictureWithTextOverlayWidget::setTextColor(const QColor &color)
{
    textColor = color;
    update();
}

/**
 * @brief Sets the outline color around the overlay text.
 * @param color The new outline color.
 */
void CardInfoPictureWithTextOverlayWidget::setOutlineColor(const QColor &color)
{
    outlineColor = color;
    update();
}

/**
 * @brief Sets the font size for the overlay text.
 * @param size The new font size.
 */
void CardInfoPictureWithTextOverlayWidget::setFontSize(const int size)
{
    fontSize = size > 0 ? size : 1;
    update();
}

/**
 * @brief Sets the alignment of the overlay text within the widget.
 * @param alignment The new text alignment.
 */
void CardInfoPictureWithTextOverlayWidget::setTextAlignment(const Qt::Alignment alignment)
{
    textAlignment = alignment;
    update();
}

void CardInfoPictureWithTextOverlayWidget::setHighlighted(bool _highlighted)
{
    if (highlighted == _highlighted) {
        return;
    }

    highlighted = _highlighted;
    update();
}

void CardInfoPictureWithTextOverlayWidget::mousePressEvent(QMouseEvent *event)
{
    emit imageClicked(event, this);
}

/**
 * @brief Paints the widget, including both the card image and the text overlay.
 * @param event The paint event.
 *
 * Draws the card image first, then overlays text on top. The text is wrapped and centered within the image.
 */
void CardInfoPictureWithTextOverlayWidget::paintEvent(QPaintEvent *event)
{
    // Call the base class's paintEvent to draw the card image
    CardInfoPictureWidget::paintEvent(event);

    QStylePainter painter(this);

    // Get the pixmap from the base class using the getter
    const QPixmap &pixmap = getResizedPixmap();
    if (pixmap.isNull()) {
        return;
    }

    // Calculate size and position for drawing
    const QSize scaledSize = pixmap.size().scaled(size(), Qt::KeepAspectRatio);
    const QPoint topLeft{(width() - scaledSize.width()) / 2, (height() - scaledSize.height()) / 2};
    const QRect pixmapRect(topLeft, scaledSize);

    if (highlighted) {
        painter.save();
        painter.setRenderHint(QPainter::Antialiasing, true);

        // Soft glow and border around the pixmap
        const int padding = 4; // glow extends a little beyond image
        QRect glowRect = pixmapRect.adjusted(-padding, -padding, padding, padding);

        QPainterPath path;
        int radius = 8; // rounded corners
        path.addRoundedRect(glowRect, radius, radius);

        // Soft outer glow
        QColor glowColor(0, 150, 255, 80); // subtle blu
        painter.setPen(QPen(glowColor, 6));
        painter.drawPath(path);

        // Thin inner border for crispness
        QColor borderColor(0, 150, 255, 200);
        painter.setPen(QPen(borderColor, 2));
        painter.drawRoundedRect(pixmapRect, radius, radius);

        painter.restore();
    }

    // If no overlay text, skip drawing the text
    if (overlayText.isEmpty()) {
        return;
    }

    // Calculate the optimal font size
    QFont font = painter.font();
    int optimalFontSize = fontSize; // Start with the user-defined font size
    const QFontMetrics baseMetrics(font);
    int textWidth = pixmapRect.width();

    // Reduce the font size until the text fits within the pixmap's width
    do {
        font.setPointSize(optimalFontSize);
        QFontMetrics fm(font);
        int currentWidth = 0;
        for (const QString &word : overlayText.split(' ')) {
            currentWidth = std::max(currentWidth, fm.horizontalAdvance(word));
        }

        if (currentWidth <= textWidth) {
            break;
        }

        --optimalFontSize;
    } while (optimalFontSize > 1);

    // Apply the calculated font size
    painter.setFont(font);

    // Wrap the text to fit within the pixmap width
    const QFontMetrics fontMetrics(font);
    QString wrappedText;
    QString currentLine;
    QStringList words = overlayText.split(' ');
    for (const QString &word : words) {
        if (fontMetrics.horizontalAdvance(currentLine + " " + word) > textWidth) {
            wrappedText += currentLine + '\n';
            currentLine = word;
        } else {
            if (!currentLine.isEmpty()) {
                currentLine += " ";
            }
            currentLine += word;
        }
    }
    wrappedText += currentLine;

    // Calculate total text block height
    int totalTextHeight = wrappedText.count('\n') * fontMetrics.height() + fontMetrics.height();

    // Adjust font size if the total text height exceeds the pixmap height
    while (totalTextHeight > pixmapRect.height() && optimalFontSize > 1) {
        --optimalFontSize;
        font.setPointSize(optimalFontSize);
        painter.setFont(font);
        const QFontMetrics newMetrics(font);
        totalTextHeight = wrappedText.count('\n') * newMetrics.height() + newMetrics.height();
    }

    // Set up the text layout options
    QTextOption textOption;
    textOption.setAlignment(textAlignment);

    // Create a text rectangle centered vertically within the pixmap rect
    auto textRect = QRect(pixmapRect.left(), pixmapRect.top(), pixmapRect.width(), totalTextHeight);
    textRect.moveTop((pixmapRect.height() - totalTextHeight) / 2 + pixmapRect.top());

    // Draw the outlined text
    drawOutlinedText(painter, textRect, wrappedText, textOption);
}

/**
 * @brief Draws text with an outline for visibility.
 * @param painter The painter to draw the text.
 * @param textRect The rectangle area to draw the text in.
 * @param text The text to display.
 * @param textOption The text layout options, such as alignment.
 *
 * Draws an outline around the text to enhance readability before drawing the main text.
 */
void CardInfoPictureWithTextOverlayWidget::drawOutlinedText(QPainter &painter,
                                                            const QRect &textRect,
                                                            const QString &text,
                                                            const QTextOption &textOption) const
{
    painter.setPen(outlineColor);
    for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
            if (dx != 0 || dy != 0) {
                QRect shiftedTextRect = textRect.translated(dx, dy);
                painter.drawText(shiftedTextRect, text, textOption);
            }
        }
    }

    // Draw the main text
    painter.setPen(textColor);
    painter.drawText(textRect, text, textOption);
}

/**
 * @brief Provides the recommended size for this widget.
 * @return The suggested widget size.
 */
QSize CardInfoPictureWithTextOverlayWidget::sizeHint() const
{
    return CardInfoPictureWidget::sizeHint();
}

/**
 * @brief Provides the minimum recommended size for this widget.
 * @return The minimum widget size.
 */
QSize CardInfoPictureWithTextOverlayWidget::minimumSizeHint() const
{
    // Same as sizeHint, but ensure that there is at least some space for the pixmap
    const QPixmap &pixmap = getResizedPixmap();
    const QSize pixmapSize = pixmap.isNull() ? QSize(0, 0) : pixmap.size();

    // Get the font metrics for the overlay text
    QFont font;
    font.setPointSize(fontSize);
    const QFontMetrics fontMetrics(font);

    // Calculate the height required for the text
    const QStringList lines = overlayText.split('\n');
    const int totalTextHeight = static_cast<int>(lines.size()) * fontMetrics.height();

    // Return the maximum width and combined height
    return {pixmapSize.width(), pixmapSize.height() + totalTextHeight};
}
