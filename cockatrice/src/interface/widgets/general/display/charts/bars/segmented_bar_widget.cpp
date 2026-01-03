#include "segmented_bar_widget.h"

#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QToolTip>

SegmentedBarWidget::SegmentedBarWidget(QString label, QVector<Segment> segments, int total, QWidget *parent)
    : QWidget(parent), label(std::move(label)), segments(std::move(segments)), total(total)
{
    setMouseTracking(true);
    setMinimumWidth(36);
    setMaximumWidth(50);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
}

QSize SegmentedBarWidget::sizeHint() const
{
    return QSize(50, 150);
}

void SegmentedBarWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    constexpr int PAD = 4;
    constexpr int LABEL_H = 20;

    int w = width();
    int h = height();

    int barX = PAD;
    int barWidth = w - PAD * 2;

    int barTop = PAD;
    int barBottom = h - PAD - LABEL_H;
    int barHeight = barBottom - barTop;

    int yCurrent = barBottom;

    // draw stacked segments
    for (int i = 0; i < segments.size(); i++) {
        const auto &seg = segments[i];

        int segHeight = total > 0 ? (seg.value * barHeight / total) : 0;
        if (segHeight < 2)
            segHeight = 2;

        QRect r(barX, yCurrent - segHeight, barWidth, segHeight);
        bool isTop = (i == segments.size() - 1);

        QLinearGradient g(r.topLeft(), r.bottomLeft());
        g.setColorAt(0, seg.color.lighter(120));
        g.setColorAt(1, seg.color.darker(110));
        p.setBrush(g);
        p.setPen(Qt::NoPen);

        if (isTop) {
            QPainterPath path;
            int radius = 6;

            int x = r.x();
            int y = r.y();
            int w = r.width();
            int h = r.height();

            path.moveTo(x, y + h);
            path.lineTo(x, y + radius);
            path.quadTo(x, y, x + radius, y);
            path.lineTo(x + w - radius, y);
            path.quadTo(x + w, y, x + w, y + radius);
            path.lineTo(x + w, y + h);
            path.lineTo(x, y + h);
            path.closeSubpath();

            p.drawPath(path);
        } else {
            p.drawRect(r);
        }

        yCurrent -= segHeight;
    }

    // draw label
    QRect labelRect(0, h - LABEL_H, w, LABEL_H);
    QFont f = p.font();
    f.setBold(true);
    p.setFont(f);
    p.setPen(Qt::black);
    p.drawText(labelRect, Qt::AlignCenter, label);
}

int SegmentedBarWidget::segmentAt(int y) const
{
    int padding = 4;
    int labelHeight = 20;
    int barHeight = height() - padding * 2 - labelHeight;
    int barTop = padding;
    int barBottom = barTop + barHeight;

    int currentTop = barBottom;

    for (int i = 0; i < segments.size(); i++) {
        int segHeight = total > 0 ? (segments[i].value * barHeight / total) : 0;
        if (segHeight < 1) {
            segHeight = 1;
        }

        int top = currentTop - segHeight;
        int bottom = currentTop;

        if (y >= top && y <= bottom)
            return i;

        currentTop -= segHeight;
    }
    return -1;
}

void SegmentedBarWidget::mouseMoveEvent(QMouseEvent *e)
{
    if (!hovered) {
        return;
    }

    int idx = segmentAt(e->pos().y());
    if (idx < 0) {
        return;
    }

    const Segment &s = segments[idx];
    QString text = QString("%1: %2 cards\n%3").arg(s.category).arg(s.value).arg(s.cards.join(", "));

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QToolTip::showText(e->globalPosition().toPoint(), text, this);
#else
    QToolTip::showText(e->globalPos(), text, this);
#endif
}
