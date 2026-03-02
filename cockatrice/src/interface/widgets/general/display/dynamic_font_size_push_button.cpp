#include "dynamic_font_size_push_button.h"

#include "dynamic_font_size_label.h"

#include <QPainter>

DynamicFontSizePushButton::DynamicFontSizePushButton(QWidget *parent) : QPushButton(parent)
{
}

void DynamicFontSizePushButton::paintEvent(QPaintEvent *event)
{
    // Call the base class paintEvent to preserve any other painting behavior
    QPushButton::paintEvent(event);

    // Adjust the font size dynamically based on the text
    QFont newFont = font();
    float fontSize = DynamicFontSizeLabel::getWidgetMaximumFontSize(this, this->text());
    newFont.setPointSizeF(fontSize);
    setFont(newFont);

    // Get painter for custom painting
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Paint the background with a linear gradient (normal state)
    QLinearGradient gradient(0, 0, 0, height());
    if (isDown()) {
        // Pressed state
        gradient.setColorAt(0, QColor(128, 128, 128));
        gradient.setColorAt(1, QColor(64, 64, 64));
    } else if (underMouse()) {
        // Hover state
        gradient.setColorAt(0, QColor(96, 96, 96));
        gradient.setColorAt(1, QColor(48, 48, 48));
    } else {
        // Normal state
        gradient.setColorAt(0, QColor(64, 64, 64)); // start color
        gradient.setColorAt(1, QColor(32, 32, 32)); // end color
    }
    painter.setBrush(gradient);
    painter.setPen(Qt::NoPen); // No border
    painter.drawRect(rect());

    // Paint the button text
    painter.setPen(QPen(textColor.isValid() ? textColor : QColor(255, 255, 255))); // Set text color
    painter.drawText(rect(), Qt::AlignCenter, text());
}

void DynamicFontSizePushButton::setTextColor(QColor color)
{
    if (color.isValid() && color != textColor) {
        textColor = color;
        update(); // Request a repaint to update the text color
    }
}

void DynamicFontSizePushButton::setTextAndColor(const QString &text, QColor color)
{
    setTextColor(color);
    setText(text);
}

QColor DynamicFontSizePushButton::getTextColor()
{
    return textColor;
}

/* Do not give any size hint as it it changes during paintEvent */
QSize DynamicFontSizePushButton::minimumSizeHint() const
{
    return QWidget::minimumSizeHint();
}

/* Do not give any size hint as it it changes during paintEvent */
QSize DynamicFontSizePushButton::sizeHint() const
{
    return QWidget::sizeHint();
}
