#include "shadow_background_label.h"

#include <QPaintEvent>
#include <QPainter>

/**
 * @class ShadowBackgroundLabel
 * @brief A QLabel with a semi-transparent black shadowed background and rounded corners.
 *
 * This label provides a styled appearance with centered white text and a translucent
 * rounded background, making it suitable for overlay or emphasis in a UI.
 */
ShadowBackgroundLabel::ShadowBackgroundLabel(QWidget *parent, const QString &text) : QLabel(parent)
{
    setAttribute(Qt::WA_TranslucentBackground);                // Allows transparency.
    setText("<font color='white'>" + text + "</font>");        ///< Ensures the text is rendered in white.
    setAlignment(Qt::AlignCenter);                             ///< Centers the text within the label.
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum); ///< Ensures minimum size constraints.
}

/**
 * @brief Handles resizing of the label.
 *
 * Ensures the label updates its appearance when resized by triggering a repaint.
 *
 * @param event The resize event containing new size information.
 */
void ShadowBackgroundLabel::resizeEvent(QResizeEvent *event)
{
    QLabel::resizeEvent(event);
    update(); // Repaint borders explicitly.
}

/**
 * @brief Custom paint event for drawing the label's background.
 *
 * Renders a semi-transparent black rounded rectangle as the background
 * and then delegates text rendering to QLabel.
 *
 * @param event The paint event for the widget.
 */
void ShadowBackgroundLabel::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    // Enable antialiasing for smoother edges.
    painter.setRenderHint(QPainter::Antialiasing, true);

    // Set semi-transparent black brush and disable border pen.
    painter.setBrush(QColor(0, 0, 0, 128)); // Semi-transparent black.
    painter.setPen(Qt::NoPen);              // No border.

    // Adjust the rectangle to account for margins.
    QRect adjustedRect = this->rect();
    int margin = contentsMargins().left(); // Assuming equal margins.
    adjustedRect.adjust(margin, margin, -margin, -margin);

    // Draw a rounded rectangle with a corner radius of 5.
    painter.drawRoundedRect(adjustedRect, 5, 5);

    // Delegate text rendering to QLabel.
    QLabel::paintEvent(event);
}
