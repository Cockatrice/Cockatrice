#include "shadow_background_label.h"

#include <QPaintEvent>
#include <QPainter>

ShadowBackgroundLabel::ShadowBackgroundLabel(QWidget *parent, const QString &text) : QLabel(parent)
{
    setAttribute(Qt::WA_TranslucentBackground); // Allows transparency
    setText("<font color='white'>" + text + "</font>");
    setAlignment(Qt::AlignCenter);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
}

void ShadowBackgroundLabel::resizeEvent(QResizeEvent *event)
{
    QLabel::resizeEvent(event);
    update(); // Repaint borders explicitly
}

void ShadowBackgroundLabel::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    // Semi-transparent background
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setBrush(QColor(0, 0, 0, 128)); // Semi-transparent black
    painter.setPen(Qt::NoPen);              // No border

    // Compute the painting rectangle accounting for margins
    QRect adjustedRect = this->rect();
    int margin = contentsMargins().left(); // Assuming equal margins
    adjustedRect.adjust(margin, margin, -margin, -margin);

    // Draw rounded rectangle
    painter.drawRoundedRect(adjustedRect, 5, 5); // Rounded corners (radius: 5)

    // Let QLabel handle text rendering
    QLabel::paintEvent(event);
}
