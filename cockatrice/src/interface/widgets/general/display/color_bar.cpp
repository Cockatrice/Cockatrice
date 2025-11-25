
#include "color_bar.h"

#include <QLinearGradient>
#include <QMouseEvent>
#include <QPainter>
#include <QToolTip>

ColorBar::ColorBar(const QMap<QString, int> &colors, QWidget *parent) : QWidget(parent), m_colors(colors)
{
    setMouseTracking(true);
}

void ColorBar::setColors(const QMap<QString, int> &colors)
{
    m_colors = colors;
    update();
}

QSize ColorBar::minimumSizeHint() const
{
    return QSize(200, 22); // Slightly taller for rounded look
}

//-------------------------------------------
// Painting
//-------------------------------------------
void ColorBar::paintEvent(QPaintEvent *)
{
    if (m_colors.isEmpty())
        return;

    int total = 0;
    for (int v : m_colors.values())
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

    // Clip to inside of the border
    p.setClipRect(bounds.adjusted(2, 2, -2, -2));

    // Sort colors by key (this ensures a predictable order)
    QList<QString> sortedKeys = m_colors.keys();
    std::sort(sortedKeys.begin(), sortedKeys.end()); // Sort alphabetically

    // Draw each color segment in the sorted order
    for (const QString &key : sortedKeys) {
        int value = m_colors[key];
        double ratio = double(value) / total;
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

//-------------------------------------------
// Hover + Tooltips
//-------------------------------------------
void ColorBar::enterEvent(QEnterEvent *)
{
    m_hover = true;
}

void ColorBar::leaveEvent(QEvent *)
{
    m_hover = false;
}

void ColorBar::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_hover || m_colors.isEmpty())
        return;

    QString text = tooltipForPosition(event->position().x());
    if (!text.isEmpty())
        QToolTip::showText(event->globalPosition().toPoint(), text, this);
}

QString ColorBar::tooltipForPosition(int x) const
{
    int total = 0;
    for (int v : m_colors.values())
        total += v;

    int pos = 0;
    for (auto it = m_colors.begin(); it != m_colors.end(); ++it) {
        double ratio = double(it.value()) / total;
        int segmentWidth = int(ratio * width());

        if (x >= pos && x < pos + segmentWidth) {
            double percent = (100.0 * it.value()) / total;
            return QString("%1: %2 cards (%.1f%)").arg(it.key()).arg(it.value()).arg(percent);
        }

        pos += segmentWidth;
    }

    return {};
}

//-------------------------------------------
// Color name mapping (expandable)
//-------------------------------------------
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