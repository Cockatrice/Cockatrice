#include "replay_timeline_widget.h"

#include "../../../client/settings/cache_settings.h"

#include <QPainter>
#include <QPainterPath>

static constexpr int BIN_LENGTH = 5000;
static constexpr int MIN_RESOLUTION_MS = 1000;

ReplayTimelineWidget::ReplayTimelineWidget(QWidget *parent) : QWidget(parent)
{
}

void ReplayTimelineWidget::setTimeline(const QList<int> &replayTimeline)
{
    histogram.clear();
    currentTime = 0;

    int binEndTime = BIN_LENGTH - 1;
    int binValue = 0;
    for (int i : replayTimeline) {
        if (i > binEndTime) {
            histogram.append(binValue);
            if (binValue > maxBinValue) {
                maxBinValue = binValue;
            }
            while (i > binEndTime + BIN_LENGTH) {
                histogram.append(0);
                binEndTime += BIN_LENGTH;
            }
            binValue = 1;
            binEndTime += BIN_LENGTH;
        } else {
            ++binValue;
        }
    }
    histogram.append(binValue);
    if (!replayTimeline.isEmpty()) {
        maxTime = replayTimeline.last();
    }

    update();
}

void ReplayTimelineWidget::paintEvent(QPaintEvent * /* event */)
{
    QPainter painter(this);
    painter.drawRect(0, 0, width() - 1, height() - 1);

    qreal binWidth = (qreal)width() / histogram.size();
    QPainterPath path;
    path.moveTo(0, height() - 1);
    for (int i = 0; i < histogram.size(); ++i) {
        path.lineTo(qRound(i * binWidth), (height() - 1) * (1.0 - (qreal)histogram[i] / maxBinValue));
    }
    path.lineTo(width() - 1, height() - 1);
    path.lineTo(0, height() - 1);
    painter.fillPath(path, Qt::black);

    const QColor barColor = QColor::fromHsv(120, 255, 255, 100);
    quint64 w = (quint64)(width() - 1) * (quint64)currentTime / maxTime;
    painter.fillRect(0, 0, static_cast<int>(w), height() - 1, barColor);
}

void ReplayTimelineWidget::mousePressEvent(QMouseEvent *event)
{
    int newTime = static_cast<int>((qint64)maxTime * (qint64)event->position().x() / width());
    emit timeClicked(newTime);
}

void ReplayTimelineWidget::setCurrentTime(int time)
{
    int newTime = qBound(0, time, maxTime);

    if (currentTime == newTime) {
        return;
    }

    bool doUpdate = currentTime / MIN_RESOLUTION_MS != newTime / MIN_RESOLUTION_MS;

    currentTime = newTime;

    if (doUpdate) {
        update();
    }
}

QSize ReplayTimelineWidget::sizeHint() const
{
    return {-1, 50};
}

QSize ReplayTimelineWidget::minimumSizeHint() const
{
    return {400, 50};
}
