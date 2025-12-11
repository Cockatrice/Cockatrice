#include "color_bar.h"

#include <QLinearGradient>
#include <QMouseEvent>
#include <QPainter>
#include <QToolTip>

ColorBar::ColorBar(const QMap<QString, int> &_colors, QWidget *parent) : QWidget(parent), colors(_colors)
{
    setMouseTracking(true);
}

void ColorBar::setColors(const QMap<QString, int> &_colors)
{
    colors = _colors;
    update();
}

QSize ColorBar::minimumSizeHint() const
{
    return QSize(200, 22);
}

void ColorBar::paintEvent(QPaintEvent *)
{
    if (colors.isEmpty())
        return;

    int total = 0;
    for (int v : colors.values())
        total += v;

    // Prevent divide-by-zero
    if (total == 0)
        return;

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    const int w = width();
    const int h = height();
    int x = 0;

    // Draw rounded border background
    QRectF bounds(0.5, 0.5, w - 1, h - 1);
    p.setPen(QPen(Qt::black, 1));
    p.setBrush(Qt::NoBrush);
    p.drawRoundedRect(bounds, 6, 6);

    // Clip to inside the border
    p.setClipRect(bounds.adjusted(2, 2, -2, -2));

    // Ensure predictable order
    QList<QString> sortedKeys = colors.keys();
    std::sort(sortedKeys.begin(), sortedKeys.end()); // Sort alphabetically

    // Draw each color segment in the sorted order
    for (const QString &key : sortedKeys) {
        int value = colors[key];
        double ratio = double(value) / total;

        if (ratio <= minRatioThreshold) {
            continue;
        }

        int segmentWidth = int(ratio * w);

        // Ensure the segment width is at least 1 to avoid degenerate rectangles
        if (segmentWidth < 1)
            segmentWidth = 1;

        QColor base = colorFromName(key);

        // Slight gradient for nicer look
        QLinearGradient grad(x, 0, x, h);
        grad.setColorAt(0, base.lighter(120));
        grad.setColorAt(1, base.darker(120));

        p.fillRect(QRect(x, 0, segmentWidth, h), grad);

        x += segmentWidth;
    }
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void ColorBar::enterEvent(QEnterEvent *event)
{
    Q_UNUSED(event);
    isHovered = true;
}
#else
void ColorBar::enterEvent(QEvent *event)
{
    Q_UNUSED(event);
    isHovered = true;
}
#endif

void ColorBar::leaveEvent(QEvent *)
{
    isHovered = false;
}

void ColorBar::mouseMoveEvent(QMouseEvent *event)
{
    if (!isHovered || colors.isEmpty())
        return;

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    int x = int(event->position().x());
    QPoint gp = event->globalPosition().toPoint();
#else
    int x = event->pos().x();
    QPoint gp = event->globalPos();
#endif

    QString text = tooltipForPosition(x);
    if (!text.isEmpty())
        QToolTip::showText(gp, text, this);
}

QString ColorBar::tooltipForPosition(int x) const
{
    int total = 0;
    for (int v : colors.values())
        total += v;

    if (total == 0)
        return {};

    int pos = 0;
    for (auto it = colors.cbegin(); it != colors.cend(); ++it) {
        const double ratio = double(it.value()) / total;
        const int segmentWidth = int(ratio * width());

        if (x >= pos && x < pos + segmentWidth) {
            const double percent = (100.0 * it.value()) / total;
            return QString("%1: %2 cards (%3%)").arg(it.key()).arg(it.value()).arg(QString::number(percent, 'f', 1));
        }

        pos += segmentWidth;
    }

    return {};
}

QColor ColorBar::colorFromName(const QString &name) const
{
    static QMap<QString, QColor> map = {
        {"R", QColor(220, 30, 30)},   {"G", QColor(40, 170, 40)}, {"U", QColor(40, 90, 200)},
        {"W", QColor(235, 235, 230)}, {"B", QColor(30, 30, 30)},
    };

    if (map.contains(name))
        return map[name];

    QColor c(name);
    if (!c.isValid())
        c = Qt::gray;

    return c;
}