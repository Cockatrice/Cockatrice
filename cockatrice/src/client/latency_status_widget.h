/**
 * @file latency_status_widget.h
 * @ingroup Client
 */

#ifndef LATENCY_STATUS_WIDGET_H
#define LATENCY_STATUS_WIDGET_H

#include <QList>
#include <QWidget>

class QLabel;
class LatencyGraphWidget;

/**
 * @brief Status bar presentation of server round-trip health.
 *
 * Combines the textual "Ping" readout with a small LatencyGraphWidget
 * sparkline of the rolling sample window. Clicking anywhere in the area opens
 * a popup with a larger graph and the numeric statistics. It closes on any
 * outside click. Stays hidden while disconnected or before any samples exist.
 * Owns all latency display state so MainWindow only needs to forward two
 * signals here.
 */
class LatencyStatusWidget : public QWidget
{
    Q_OBJECT
public:
    explicit LatencyStatusWidget(QWidget *parent = nullptr);

public slots:
    void updateStats(int lastMs, int medianMs, int p95Ms, int maxMs, int sampleCount);
    void updateSamples(const QList<int> &samplesMs);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void togglePopup();
    QString statsText(int lastMs, int medianMs, int p95Ms, int maxMs, int sampleCount) const;

    QLabel *pingLabel = nullptr;
    LatencyGraphWidget *latencyGraph = nullptr;
    QWidget *popup = nullptr;
    LatencyGraphWidget *detailGraph = nullptr;
    QLabel *detailLabel = nullptr;
    QList<int> latestSamples;
};

#endif
