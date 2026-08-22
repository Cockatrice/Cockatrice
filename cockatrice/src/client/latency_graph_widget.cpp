/**
 * @file latency_graph_widget.cpp
 * @ingroup Client
 */

#include "latency_graph_widget.h"

#include <QPainter>

LatencyGraphWidget::LatencyGraphWidget(QWidget *parent) : QWidget(parent)
{
}

void LatencyGraphWidget::setSamples(const QList<int> &samplesMs)
{
    samples = samplesMs;
    update();
}

void LatencyGraphWidget::paintEvent(QPaintEvent * /* event */)
{
    if (samples.isEmpty()) {
        return;
    }

    QPainter painter(this);

    // Heights are relative to the window's own worst sample (floored at
    // MinScaleMs) so the shape of the variance stays readable even when every
    // value is small.
    qint64 heightScaleMs = MinScaleMs;
    for (int sample : samples) {
        heightScaleMs = qMax(heightScaleMs, static_cast<qint64>(sample));
    }

    const qreal widthPerBar = static_cast<qreal>(width()) / samples.size();
    for (int i = 0; i < samples.size(); ++i) {
        const qreal heightRatio = qBound(0.0, static_cast<double>(samples.at(i)) / heightScaleMs, 1.0);
        const qreal barHeight = heightRatio * height();

        // Colors follow an absolute quality ramp: a steady good ping stays
        // green no matter how uniform the window is.
        const qreal colorRatio = qBound(0.0, static_cast<double>(samples.at(i)) / ColorScaleMs, 1.0);
        QColor color;
        color.setHsv(qRound(120.0 * (1.0 - colorRatio)), 255, 255);

        const QRectF bar(static_cast<qreal>(i) * widthPerBar + 1.0,
                         static_cast<qreal>(height()) - barHeight,
                         qMax(1.0, widthPerBar - 2.0), barHeight);
        painter.fillRect(bar, color);
    }
}
