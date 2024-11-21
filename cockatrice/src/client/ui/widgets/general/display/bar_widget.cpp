#include "bar_widget.h"

#include <utility>

BarWidget::BarWidget(QString label, int value, int total, QWidget *parent)
    : QWidget(parent), label(std::move(label)), value(value), total(total)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

QSize BarWidget::sizeHint() const
{
    QFontMetrics metrics(font());
    int labelHeight = metrics.height();
    int valueHeight = metrics.height();

    // Calculate the height based on total and value
    int barHeight = total > 0 ? total * 2 : 20;                   // Adjust factor as necessary for visual size
    int totalHeight = barHeight + labelHeight + valueHeight + 30; // Extra space for padding
    return QSize(40, totalHeight);                                // Width is fixed, height is calculated
}

void BarWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Calculate the full bar width and value-specific bar width
    int fullBarWidth = total; // Full bar (sum of all entries)
    int valueBarWidth = (total > 0) ? (value * 100 / total) : 0;

    // Draw the full bar background (gray)
    painter.setBrush(QColor(200, 200, 200)); // Gray background for full bar
    painter.drawRect(0, 10, 20, fullBarWidth);

    // Draw the value-specific bar (blue)
    painter.setBrush(QColor(100, 150, 255)); // Blue for the value bar
    painter.drawRect(0, fullBarWidth + 10, 20, -valueBarWidth);

    // Draw the CMC label
    painter.setPen(Qt::white);
    QRect textRect(0, fullBarWidth + 10, 20, 30); // Define a rectangle for text
    painter.drawText(textRect, Qt::AlignCenter, label);

    painter.setPen(Qt::black);
    QRect valueRect(0, 10, 20, fullBarWidth); // Define a rectangle for text
    painter.drawText(valueRect, Qt::AlignCenter, QString::number(value));

    (void)event; // Suppress unused parameter warning
}
