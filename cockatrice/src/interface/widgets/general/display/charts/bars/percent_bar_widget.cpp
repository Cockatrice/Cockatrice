#include "percent_bar_widget.h"

PercentBarWidget::PercentBarWidget(QWidget *parent, double initialValue) : QWidget(parent), valueToDisplay(initialValue)
{
    setMinimumSize(50, 10);
}

void PercentBarWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    QRect rect = this->rect();

    const int midX = rect.width() / 2;
    const int height = rect.height();

    // Draw background border (no fill)
    painter.setPen(QPen(Qt::black, 1));
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(rect.adjusted(0, 0, -1, -1)); // Avoid right/bottom overflow

    const double halfWidth = rect.width() / 2.0;

    const int barLength = static_cast<int>((qAbs(valueToDisplay) / 100.0) * halfWidth);

    QRect fillRect;
    if (valueToDisplay > 0.0) {
        fillRect = QRect(midX, 0, barLength, height);
        painter.fillRect(fillRect, Qt::green);
    } else if (valueToDisplay < 0.0) {
        fillRect = QRect(midX - barLength, 0, barLength, height);
        painter.fillRect(fillRect, Qt::red);
    }

    // Draw center line at 0
    painter.fillRect(midX - 1, 0, 3, height, Qt::white);

    // Draw tick marks every 10%
    const int tickHeight = 4;

    for (int percent = -100; percent <= 100; percent += 10) {
        int x = midX + static_cast<int>((percent / 100.0) * halfWidth);
        painter.drawLine(x, height - tickHeight, x, height);
    }
}
