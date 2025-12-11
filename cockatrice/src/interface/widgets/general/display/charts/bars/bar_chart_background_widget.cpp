#include "bar_chart_background_widget.h"

BarChartBackgroundWidget::BarChartBackgroundWidget(QWidget *parent) : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

QSize BarChartBackgroundWidget::sizeHint() const
{
    return QSize(100, 150);
}

void BarChartBackgroundWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    constexpr int PAD = 4;
    constexpr int LABEL_H = 20;

    int left = 46; // axis space + internal padding
    int right = width() - PAD;
    int top = PAD;
    int bottom = height() - PAD - LABEL_H;

    int barAreaHeight = bottom - top;
    int barAreaWidth = right - left;

    p.fillRect(QRect(left, top, barAreaWidth, barAreaHeight), QColor(250, 250, 250));

    int ticks = 5;
    for (int i = 0; i <= ticks; i++) {
        float r = float(i) / ticks;
        int y = bottom - r * barAreaHeight;

        p.setPen(QPen(QColor(180, 180, 180, 120), 1));
        p.drawLine(left, y, right, y);

        p.setPen(Qt::black);
        p.drawText(left - 35, y - 6, 32, 12, Qt::AlignRight | Qt::AlignVCenter, QString::number(int(r * highest)));
    }
}
