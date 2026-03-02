#include "bar_chart_widget.h"

#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QToolTip>

BarChartWidget::BarChartWidget(QWidget *parent) : QWidget(parent)
{
    setMouseTracking(true);
}

void BarChartWidget::setBars(const QVector<BarData> &newBars)
{
    bars = newBars;
    update();
}

void BarChartWidget::setHighest(int h)
{
    highest = qMax(1, h);
    update();
}

QSize BarChartWidget::sizeHint() const
{
    return QSize(300, 200);
}

QSize BarChartWidget::minimumSizeHint() const
{
    return QSize(300, 50);
}

void BarChartWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    constexpr int PAD = 4;
    constexpr int LABEL_H = 20;

    int w = width();
    int h = height();

    int left = 46;
    int right = w - PAD;
    int top = PAD;
    int bottom = h - PAD - LABEL_H;

    int barAreaHeight = bottom - top;
    int barAreaWidth = right - left;

    int barCount = bars.size();
    if (barCount == 0)
        return;

    int spacing = 6;
    int barWidth = (barAreaWidth - (barCount - 1) * spacing) / barCount;

    // background
    p.fillRect(QRect(left, top, barAreaWidth, barAreaHeight), QColor(250, 250, 250));

    // y-axis ticks
    int ticks = 5;
    // qInfo() << "Tick Positions ";
    for (int i = 0; i <= ticks; i++) {
        float r = float(i) / ticks;
        int tickVal = i * highest / ticks; // integer value of tick
        int y = bottom - (tickVal * barAreaHeight / highest);

        // qInfo() << "Tick" << i << "value" << int(r * highest) << "y" << y;

        p.setPen(QPen(QColor(180, 180, 180, 120), 1));
        p.drawLine(left, y, right, y);

        p.setPen(Qt::black);
        p.drawText(left - 35, y - 6, 32, 12, Qt::AlignRight | Qt::AlignVCenter, QString::number(int(r * highest)));
    }

    // draw bars
    // qInfo() << "Bar Segments";
    int drawWidth = barWidth / 4;             // 1/4 of allocated width
    int xOffset = (barWidth - drawWidth) / 2; // center the narrow bar

    for (int i = 0; i < barCount; i++) {
        const BarData &bar = bars[i];
        int x = left + i * (barWidth + spacing) + xOffset; // shift to center
        int yCurrent = bottom;

        for (int j = 0; j < bar.segments.size(); j++) {
            const auto &seg = bar.segments[j];
            int segHeight = (seg.value * barAreaHeight / highest);
            if (segHeight < 2 && seg.value > 0)
                segHeight = 2;

            int topY = yCurrent - segHeight;

            QRect r(x, topY, drawWidth, segHeight); // use drawWidth instead of barWidth
            bool isTop = (j == bar.segments.size() - 1);

            QLinearGradient g(r.topLeft(), r.bottomLeft());
            g.setColorAt(0, seg.color.lighter(120));
            g.setColorAt(1, seg.color.darker(110));
            p.setBrush(g);
            p.setPen(Qt::NoPen);

            if (isTop) {
                QPainterPath path;
                int radius = 6;

                int bx = r.x();
                int by = r.y();
                int bw = r.width();
                int bh = r.height();

                path.moveTo(bx, by + bh);
                path.lineTo(bx, by + radius);
                path.quadTo(bx, by, bx + radius, by);
                path.lineTo(bx + bw - radius, by);
                path.quadTo(bx + bw, by, bx + bw, by + radius);
                path.lineTo(bx + bw, by + bh);
                path.lineTo(bx, by + bh);
                path.closeSubpath();

                p.drawPath(path);
            } else {
                p.drawRect(r);
            }

            yCurrent -= segHeight;
        }

        // draw label below bar
        QRect labelRect(left + i * (barWidth + spacing), bottom, barWidth, LABEL_H);
        QFont f = p.font();
        f.setBold(true);
        p.setFont(f);
        p.setPen(Qt::black);
        p.drawText(labelRect, Qt::AlignCenter, bar.label);
    }
}

void BarChartWidget::leaveEvent(QEvent *)
{
    hoveredBar = -1;
    hoveredSegment = -1;
    QToolTip::hideText();
}

void BarChartWidget::mouseMoveEvent(QMouseEvent *e)
{
    if (bars.isEmpty()) {
        return;
    }

    constexpr int PAD = 4;
    constexpr int LABEL_H = 20;
    int w = width();
    int h = height();
    int left = 46;
    int right = w - PAD;
    int top = PAD;
    int bottom = h - PAD - LABEL_H;
    int barAreaHeight = bottom - top;

    int barCount = bars.size();
    int spacing = 6;
    int barWidth = (right - left - (barCount - 1) * spacing) / barCount;

    // find hovered bar
    int mx = e->pos().x();
    hoveredBar = -1;
    for (int i = 0; i < barCount; i++) {
        int x0 = left + i * (barWidth + spacing);
        if (mx >= x0 && mx <= x0 + barWidth) {
            hoveredBar = i;
            break;
        }
    }
    if (hoveredBar < 0) {
        return;
    }

    // find hovered segment
    int yCurrent = bottom;
    const auto &segments = bars[hoveredBar].segments;
    hoveredSegment = -1;
    for (int i = 0; i < segments.size(); i++) {
        const auto &seg = segments[i];
        int segHeight = (seg.value * barAreaHeight / highest);
        if (segHeight < 2 && seg.value > 0)
            segHeight = 2;

        int topY = yCurrent - segHeight;
        int bottomY = yCurrent;
        if (e->pos().y() >= topY && e->pos().y() <= bottomY) {
            hoveredSegment = i;
            break;
        }
        yCurrent -= segHeight;
    }

    if (hoveredSegment >= 0) {
        const auto &s = segments[hoveredSegment];
        QString text = QString("%1: %2 cards\n\n%3").arg(s.category).arg(s.value).arg(s.cards.join("\n"));
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        QToolTip::showText(e->globalPosition().toPoint(), text, this);
#else
        QToolTip::showText(e->globalPos(), text, this);
#endif
    } else {
        QToolTip::hideText();
    }
}
