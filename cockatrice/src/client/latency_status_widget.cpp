/**
 * @file latency_status_widget.cpp
 * @ingroup Client
 */

#include "latency_status_widget.h"

#include "latency_graph_widget.h"

#include <QHBoxLayout>
#include <QLabel>

LatencyStatusWidget::LatencyStatusWidget(QWidget *parent) : QWidget(parent)
{
    pingLabel = new QLabel(this);
    pingLabel->setAccessibleName(tr("Ping"));

    latencyGraph = new LatencyGraphWidget(this);
    latencyGraph->setFixedSize(90, 14);

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(4);
    layout->addWidget(latencyGraph);
    layout->addWidget(pingLabel);

    hide();
}

void LatencyStatusWidget::updateStats(int lastMs, int medianMs, int p95Ms, int maxMs, int sampleCount)
{
    if (sampleCount == 0) {
        hide();
        return;
    }

    const QString stats = tr("Connection quality over the last %n sample(s):", "", sampleCount) + "\n" +
                          tr("Last: %1 ms").arg(lastMs) + "\n" + tr("Median: %1 ms").arg(medianMs) + "\n" +
                          tr("95th percentile: %1 ms").arg(p95Ms) + "\n" + tr("Maximum: %1 ms").arg(maxMs);

    pingLabel->setText(tr("Ping: %1 ms").arg(lastMs));
    pingLabel->setToolTip(stats);
    pingLabel->setAccessibleDescription(stats);
    show();
}

void LatencyStatusWidget::updateSamples(const QList<int> &samplesMs)
{
    latencyGraph->setSamples(samplesMs);
}
