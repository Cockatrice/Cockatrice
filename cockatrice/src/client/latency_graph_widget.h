/**
 * @file latency_graph_widget.h
 * @ingroup Client
 */

#ifndef LATENCY_GRAPH_WIDGET_H
#define LATENCY_GRAPH_WIDGET_H

#include <QList>
#include <QWidget>

/**
 * @brief Bar graph of recent network round-trip samples.
 *
 * Draws one bar per sample, oldest on the left. Bar height is relative to the
 * window's own scale so the shape of the variance stays readable, while bar
 * color maps each sample onto an absolute quality ramp (green at rest through
 * red at ColorScaleMs) so a steady good ping never looks alarming. Size
 * agnostic: the status bar embeds a small instance while the latency detail
 * popup shows a large one.
 */
class LatencyGraphWidget : public QWidget
{
    Q_OBJECT
public:
    explicit LatencyGraphWidget(QWidget *parent = nullptr);

    /// Sample in milliseconds that maps to a fully red bar.
    static constexpr qint64 ColorScaleMs = 500;

    void setSamples(const QList<int> &samplesMs);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    /// Floor of the vertical scale in milliseconds. Keeps small windows readable.
    static constexpr qint64 MinScaleMs = 100;

    QList<int> samples;
};

#endif
