#include "color_pie.h"

#include "../../../../../essence_chart_colors.h"

#include <QBrush>
#include <QFontMetrics>
#include <QMouseEvent>
#include <QPainter>
#include <QPen>
#include <QPoint>
#include <QPointF>
#include <QRadialGradient>
#include <QRectF>
#include <QSize>
#include <QToolTip>
#include <QtGlobal>
#include <QtMath>
#include <cmath>
#include <qnamespace.h>
#include <utility>

ColorPie::ColorPie(const QMap<QString, int> &_colors, QWidget *parent)
    : QWidget(parent), colors(EssenceCharts::ordered(_colors))
{
    setMouseTracking(true);
}

void ColorPie::setColors(const QMap<QString, int> &_colors)
{
    colors = EssenceCharts::ordered(_colors);
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
    for (const auto &pair : colors) {
        total += pair.second;
    }

    if (total == 0) {
        return;
    }

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    int w = width();
    int h = height();
    int size = qMin(w, h) - 40;
    QRectF rect((w - size) / 2.0, (h - size) / 2.0, size, size);

    // Border
    p.setPen(QPen(Qt::black, 1));
    p.setBrush(Qt::NoBrush);
    p.drawEllipse(rect);

    double startAngle = 0.0;

    for (const auto &[key, value] : colors) {
        double ratio = double(value) / total;
        if (ratio <= minRatioThreshold) {
            continue;
        }

        double spanAngle = ratio * 360.0;

        QColor base = colorFromName(key);

        QRadialGradient grad(rect.center(), size / 2);
        grad.setColorAt(0, base.lighter(130));
        grad.setColorAt(1, base.darker(130));

        p.setBrush(grad);
        p.setPen(Qt::NoPen);
        p.drawPie(rect, int(startAngle * 16), int(spanAngle * 16));

        // Percent label
        double midAngle = startAngle + spanAngle / 2.0;
        double rad = qDegreesToRadians(midAngle);
        double labelRadius = size / 2 + 15;
        QPointF center = rect.center();
        QPointF labelPos(center.x() + labelRadius * qCos(rad), center.y() - labelRadius * qSin(rad));

        QString label = QString("%1%").arg(int(ratio * 100 + 0.5));

        QFontMetrics fm(p.font());
        int labelWidth = fm.horizontalAdvance(label);
        QRectF textRect(labelPos.x() - labelWidth / 2.0, labelPos.y() - fm.height() / 2.0, labelWidth, fm.height());

        p.setPen(Qt::black);
        p.drawText(textRect, Qt::AlignCenter, label);

        startAngle += spanAngle;
    }
}

void ColorPie::enterEvent(QEnterEvent *event)
{
    Q_UNUSED(event);
    isHovered = true;
}

void ColorPie::leaveEvent(QEvent *)
{
    isHovered = false;
}

void ColorPie::mouseMoveEvent(QMouseEvent *event)
{
    if (!isHovered || colors.isEmpty()) {
        return;
    }

    QPoint p = event->position().toPoint();
    QPoint gp = event->globalPosition().toPoint();

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
    for (const auto &pair : colors) {
        total += pair.second;
    }

    if (total == 0) {
        return {};
    }

    int w = width();
    int h = height();
    int size = qMin(w, h) - 40;
    QPointF center(w / 2.0, h / 2.0);

    QPointF v = pt - center;
    double distance = std::hypot(v.x(), v.y());
    if (distance > size / 2.0) {
        return {};
    }

    double angle = std::atan2(-v.y(), v.x()) * 180.0 / M_PI;
    if (angle < 0) {
        angle += 360.0;
    }

    double acc = 0.0;

    for (const auto &[key, value] : colors) {
        double ratio = double(value) / total;
        if (ratio <= minRatioThreshold) {
            continue;
        }

        double span = ratio * 360.0;

        if (angle >= acc && angle < acc + span) {
            double percent = (100.0 * value) / total;
            return QString("%1: %2 cards (%3%)").arg(key).arg(value).arg(QString::number(percent, 'f', 1));
        }

        acc += span;
    }

    return {};
}

QColor ColorPie::colorFromName(const QString &name) const
{
    return EssenceCharts::colorFor(name);
}
