/**
 * @file latency_status_widget.cpp
 * @ingroup Client
 */

#include "latency_status_widget.h"

#include "latency_graph_widget.h"

#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

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

    // Clicking anywhere in the area opens the detail view.
    for (QObject *child : QList<QObject *>{pingLabel, latencyGraph}) {
        child->installEventFilter(this);
    }
    setCursor(Qt::PointingHandCursor);

    hide();
}

void LatencyStatusWidget::updateStats(int lastMs, int medianMs, int p95Ms, int maxMs, int sampleCount)
{
    if (sampleCount == 0) {
        hide();
        return;
    }

    const QString stats = statsText(lastMs, medianMs, p95Ms, maxMs, sampleCount);

    pingLabel->setText(tr("Ping: %1 ms").arg(lastMs));
    pingLabel->setToolTip(stats);
    pingLabel->setAccessibleDescription(stats);
    if (popup && popup->isVisible() && detailLabel) {
        detailLabel->setText(stats);
    }
    show();
}

void LatencyStatusWidget::updateSamples(const QList<int> &samplesMs)
{
    latestSamples = samplesMs;
    latencyGraph->setSamples(samplesMs);
    if (popup && popup->isVisible() && detailGraph) {
        detailGraph->setSamples(samplesMs);
    }
}

bool LatencyStatusWidget::eventFilter(QObject *watched, QEvent *event)
{
    if ((watched == pingLabel || watched == latencyGraph) && event->type() == QEvent::MouseButtonPress) {
        togglePopup();
        return true;
    }
    return QWidget::eventFilter(watched, event);
}

void LatencyStatusWidget::togglePopup()
{
    if (!popup) {
        popup = new QWidget(this, Qt::Popup | Qt::FramelessWindowHint);
        auto *layout = new QVBoxLayout(popup);
        layout->setContentsMargins(8, 8, 8, 8);

        detailLabel = new QLabel(popup);
        detailLabel->setAccessibleName(tr("Connection latency details"));
        detailLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

        detailGraph = new LatencyGraphWidget(popup);
        detailGraph->setFixedSize(280, 80);

        layout->addWidget(detailLabel, 0, Qt::AlignLeft);
        layout->addWidget(detailGraph, 0, Qt::AlignHCenter);
    }

    if (popup->isVisible()) {
        popup->hide();
        return;
    }

    // Qt::Popup closes itself on any outside click, so just position and show.
    if (latestSamples.isEmpty()) {
        return;
    }
    detailGraph->setSamples(latestSamples);
    detailLabel->setText(pingLabel->toolTip());
    popup->adjustSize();
    const QPoint anchor = mapToGlobal(QPoint(width() / 2, 0));
    popup->move(anchor.x() - popup->width() / 2, anchor.y() - popup->height() - 6);
    popup->show();
}

QString LatencyStatusWidget::statsText(int lastMs, int medianMs, int p95Ms, int maxMs, int sampleCount) const
{
    return tr("Connection quality over the last %n sample(s):", "", sampleCount) + "\n" +
           tr("Last: %1 ms").arg(lastMs) + "\n" + tr("Median: %1 ms").arg(medianMs) + "\n" +
           tr("95th percentile: %1 ms").arg(p95Ms) + "\n" + tr("Maximum: %1 ms").arg(maxMs);
}
