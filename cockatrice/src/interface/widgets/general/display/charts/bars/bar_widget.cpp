#include "bar_widget.h"

#include <QFontMetrics>
#include <QPainter>

BarWidget::BarWidget(QString label, int value, int total, QColor barColor, QWidget *parent)
    : QWidget(parent), label(std::move(label)), value(value), total(total), barColor(barColor)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

QSize BarWidget::sizeHint() const
{
    QFontMetrics metrics(font());
    int labelHeight = metrics.height();
    int valueHeight = metrics.height();

    // Calculate the height dynamically based on the total
    int barHeight = (total > 0) ? (value * 200 / total) : 20;     // Scale height proportionally
    int totalHeight = barHeight + labelHeight + valueHeight + 30; // Extra space for text
    return QSize(60, totalHeight);                                // Allow width to expand
}

void BarWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    int widgetWidth = width();
    int widgetHeight = height();

    // Calculate bar dimensions
    int barWidth = widgetWidth * 0.8;      // Use 80% of the available width
    int fullBarHeight = widgetHeight - 40; // Leave space for labels
    int valueBarHeight = (total > 0) ? (value * fullBarHeight / total) : 0;

    // Draw full bar background (gray)
    painter.setBrush(QColor(200, 200, 200));
    painter.drawRect((widgetWidth - barWidth) / 2, 10, barWidth, fullBarHeight);

    // Draw the value-specific bar using the assigned color
    painter.setBrush(barColor);
    painter.drawRect((widgetWidth - barWidth) / 2, 10 + fullBarHeight - valueBarHeight, barWidth, valueBarHeight);

    // Draw the CMC label
    painter.setPen(Qt::white);
    QRect textRect(0, widgetHeight - 30, widgetWidth, 20);
    painter.drawText(textRect, Qt::AlignCenter, label);

    // Draw the value count
    painter.setPen(Qt::black);
    QRect valueRect(0, 10, widgetWidth, 20);
    painter.drawText(valueRect, Qt::AlignCenter, QString::number(value));

    (void)event; // Suppress unused parameter warning
}
