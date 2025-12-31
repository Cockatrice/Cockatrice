#include "color_pie.h"

#include <QMouseEvent>
#include <QPainter>
#include <QToolTip>
#include <QtMath>

ColorPie::ColorPie(const QMap<QString, int> &_colors, QWidget *parent) : QWidget(parent), colors(_colors)
{
    setMouseTracking(true);
}

void ColorPie::setColors(const QMap<QString, int> &_colors)
{
    colors = _colors;
    update();
}

QSize ColorPie::minimumSizeHint() const
{
    return QSize(200, 200);
}

void ColorPie::paintEvent(QPaintEvent *)
{
    if (colors.isEmpty()) {
        return;
    }

    int total = 0;
    for (int v : colors.values()) {
        total += v;
    }

    if (total == 0) {
        return;
    }

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    int w = width();
    int h = height();
    int size = qMin(w, h) - 40; // leave space for labels
    QRectF rect((w - size) / 2.0, (h - size) / 2.0, size, size);

    // Draw border
    p.setPen(QPen(Qt::black, 1));
    p.setBrush(Qt::NoBrush);
    p.drawEllipse(rect);

    // Sorted keys for predictable order
    QList<QString> sortedKeys = colors.keys();
    std::sort(sortedKeys.begin(), sortedKeys.end());

    double startAngle = 0.0;

    for (const QString &key : sortedKeys) {
        int value = colors[key];
        double ratio = double(value) / total;

        if (ratio <= minRatioThreshold) {
            continue;
        }

        double spanAngle = ratio * 360.0;

        QColor base = colorFromName(key);

        // Gradient
        QRadialGradient grad(rect.center(), size / 2);
        grad.setColorAt(0, base.lighter(130));
        grad.setColorAt(1, base.darker(130));
        p.setBrush(grad);
        p.setPen(Qt::NoPen);

        // Draw slice
        p.drawPie(rect, int(startAngle * 16), int(spanAngle * 16));

        // Draw percent label
        double midAngle = startAngle + spanAngle / 2;
        double rad = qDegreesToRadians(midAngle);
        double labelRadius = size / 2 + 15; // slightly outside the pie
        QPointF center = rect.center();
        QPointF labelPos(center.x() + labelRadius * qCos(rad), center.y() - labelRadius * qSin(rad));

        QString label = QString("%1%").arg(int(ratio * 100 + 0.5));

        QFontMetrics fm(p.font());
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
        int labelWidth = fm.horizontalAdvance(label);
#else
        int labelWidth = fm.width(label);
#endif
        QRectF textRect(labelPos.x() - labelWidth / 2.0, labelPos.y() - fm.height() / 2.0, labelWidth, fm.height());

        p.setPen(Qt::black);
        p.drawText(textRect, Qt::AlignCenter, label);

        startAngle += spanAngle;
    }
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void ColorPie::enterEvent(QEnterEvent *event)
{
    Q_UNUSED(event);
    isHovered = true;
}
#else
void ColorPie::enterEvent(QEvent *event)
{
    Q_UNUSED(event);
    isHovered = true;
}
#endif

void ColorPie::leaveEvent(QEvent *)
{
    isHovered = false;
}

void ColorPie::mouseMoveEvent(QMouseEvent *event)
{
    if (!isHovered || colors.isEmpty()) {
        return;
    }

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QPoint p = event->position().toPoint();
    QPoint gp = event->globalPosition().toPoint();
#else
    QPoint p = event->pos();
    QPoint gp = event->globalPos();
#endif

    QString text = tooltipForPoint(p);
    if (!text.isEmpty()) {
        QToolTip::showText(gp, text, this);
    }
}

QString ColorPie::tooltipForPoint(const QPoint &pt) const
{
    if (colors.isEmpty()) {
        return {};
    }

    int total = 0;
    for (int v : colors.values())
        total += v;
    if (total == 0)
        return {};

    int w = width();
    int h = height();
    int size = qMin(w, h) - 40;
    QPointF center(w / 2.0, h / 2.0);

    QPointF v = pt - center;
    double distance = std::sqrt(v.x() * v.x() + v.y() * v.y());
    if (distance > size / 2.0)
        return {}; // outside pie

    double angle = std::atan2(-v.y(), v.x()) * 180.0 / M_PI;
    if (angle < 0) {
        angle += 360.0;
    }

    double acc = 0.0;

    QList<QString> keys = colors.keys();
    std::sort(keys.begin(), keys.end());

    for (const QString &key : keys) {
        double span = (double(colors[key]) / total) * 360.0;

        if (angle >= acc && angle < acc + span) {
            double percent = (100.0 * colors[key]) / total;
            return QString("%1: %2 cards (%3%)").arg(key).arg(colors[key]).arg(QString::number(percent, 'f', 1));
        }
        acc += span;
    }

    return {};
}

QColor ColorPie::colorFromName(const QString &name) const
{
    static QMap<QString, QColor> map = {
        {"R", QColor(220, 30, 30)},   {"G", QColor(40, 170, 40)}, {"U", QColor(40, 90, 200)},
        {"W", QColor(235, 235, 230)}, {"B", QColor(30, 30, 30)},
    };

    if (map.contains(name)) {
        return map[name];
    }

    QColor c(name);
    if (!c.isValid()) {
        c = Qt::gray;
    }

    return c;
}