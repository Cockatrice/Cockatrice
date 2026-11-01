#include "replay_timeline_widget.h"

#include <QBrush>
#include <QColor>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPointF>
#include <QSize>
#include <QtGlobal>
#include <qnamespace.h>
#include <qnumeric.h>

static constexpr int BIN_LENGTH = 5000;

ReplayTimelineWidget::ReplayTimelineWidget(QWidget *parent) : QWidget(parent)
{
}

void ReplayTimelineWidget::setTimeline(const QList<int> &replayTimeline)
{
    histogram.clear();
    currentTime = 0;
    maxBinValue = 1;
    maxTime = 0;

    for (int time : replayTimeline) {
        maxTime = qMax(maxTime, time);
    }
    if (!replayTimeline.isEmpty()) {
        histogram.fill(0, maxTime / BIN_LENGTH + 1);
        for (int time : replayTimeline) {
            const int count = ++histogram[qMax(0, time) / BIN_LENGTH];
            maxBinValue = qMax(maxBinValue, count);
        }
    }

    update();
}

void ReplayTimelineWidget::paintEvent(QPaintEvent * /* event */)
{
    QPainter painter(this);
    const int contentWidth = width() - 1;
    const int contentHeight = height() - 1;
    if (contentWidth <= 0 || contentHeight <= 0) {
        return;
    }
    painter.drawRect(0, 0, contentWidth, contentHeight);

    // An empty replay can be painted before setTimeline(), or after replacing
    // another replay. A replay containing only time-zero events is also valid.
    if (histogram.isEmpty()) {
        return;
    }

    // The final five-second bin can be partial. Align histogram activity with
    // the same actual time span used by clicking and the playback indicator.
    QPainterPath path;
    path.moveTo(0, contentHeight);
    for (int i = 0; i < histogram.size(); ++i) {
        const qreal y = contentHeight * (1.0 - static_cast<qreal>(histogram[i]) / maxBinValue);
        const qint64 startTime = static_cast<qint64>(i) * BIN_LENGTH;
        const qint64 endTime = qMin<qint64>(startTime + BIN_LENGTH, maxTime);
        const qreal startX = maxTime > 0 ? static_cast<qreal>(contentWidth) * startTime / maxTime : 0;
        const qreal endX = maxTime > 0 ? static_cast<qreal>(contentWidth) * endTime / maxTime : contentWidth;
        path.lineTo(startX, y);
        path.lineTo(endX, y);
    }
    path.lineTo(contentWidth, contentHeight);
    path.lineTo(0, contentHeight);
    painter.fillPath(path, Qt::black);

    const QColor barColor = QColor::fromHsv(120, 255, 255, 100);
    const int progressWidth =
        maxTime > 0 ? static_cast<int>(static_cast<qint64>(contentWidth) * currentTime / maxTime) : 0;
    painter.fillRect(0, 0, progressWidth, contentHeight, barColor);
}

void ReplayTimelineWidget::mousePressEvent(QMouseEvent *event)
{
    // Use the same drawable span as the progress bar so its right edge reaches
    // the final event. Clamp dragged/outside coordinates and narrow widgets.
    const int contentWidth = width() - 1;
    const qreal position = qBound<qreal>(0, event->position().x(), qMax(0, contentWidth));
    const int newTime = contentWidth > 0 ? qRound(maxTime * (position / contentWidth)) : 0;
    emit timeClicked(newTime);
}

void ReplayTimelineWidget::setCurrentTime(int time)
{
    int newTime = qBound(0, time, maxTime);

    if (currentTime == newTime) {
        return;
    }

    currentTime = newTime;
    // Exact event seeks can remain within one second. Repaint their position
    // immediately; QWidget coalesces redundant paint requests itself.
    update();
}

QSize ReplayTimelineWidget::sizeHint() const
{
    return {-1, 50};
}

QSize ReplayTimelineWidget::minimumSizeHint() const
{
    return {400, 50};
}
