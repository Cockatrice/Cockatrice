#include "replay_timeline_widget.h"
#include <QPainter>
#include <QPalette>
#include <QTimer>
#include <cmath>
#ifdef _WIN32
#include "round.h"
#endif /* _WIN32 */

ReplayTimelineWidget::ReplayTimelineWidget(QWidget *parent)
    : QWidget(parent), maxBinValue(1), maxTime(1), timeScaleFactor(1.0), currentTime(0), currentEvent(0)
{
    replayTimer = new QTimer(this);
    connect(replayTimer, SIGNAL(timeout()), this, SLOT(replayTimerTimeout()));
}

const int ReplayTimelineWidget::binLength = 5000;

void ReplayTimelineWidget::setTimeline(const QList<int> &_replayTimeline)
{
    replayTimeline = _replayTimeline;
    histogram.clear();
    int binEndTime = binLength - 1;
    int binValue = 0;
    for (int i = 0; i < replayTimeline.size(); ++i) {
        if (replayTimeline[i] > binEndTime) {
            histogram.append(binValue);
            if (binValue > maxBinValue)
                maxBinValue = binValue;
            while (replayTimeline[i] > binEndTime + binLength) {
                histogram.append(0);
                binEndTime += binLength;
            }
            binValue = 1;
            binEndTime += binLength;
        } else
            ++binValue;
    }
    histogram.append(binValue);
    if (!replayTimeline.isEmpty())
        maxTime = replayTimeline.last();
    
    update();
}

void ReplayTimelineWidget::paintEvent(QPaintEvent * /* event */)
{
    QPainter painter(this);
    painter.drawRect(0, 0, width() - 1, height() - 1);
    
    qreal binWidth = (qreal) width() / histogram.size();
    QPainterPath path;
    path.moveTo(0, height() - 1);
    for (int i = 0; i < histogram.size(); ++i)
        path.lineTo(round(i * binWidth), (height() - 1) * (1.0 - (qreal) histogram[i] / maxBinValue));
    path.lineTo(width() - 1, height() - 1);
    path.lineTo(0, height() - 1);
    painter.fillPath(path, Qt::black);
    
    const QColor barColor = QColor::fromHsv(120, 255, 255, 100);
    quint64 w = (quint64)(width() - 1) * (quint64) currentTime / maxTime;
    painter.fillRect(0, 0, w, height() - 1, barColor);
}

QSize ReplayTimelineWidget::sizeHint() const
{
    return QSize(-1, 50);
}

QSize ReplayTimelineWidget::minimumSizeHint() const
{
    return QSize(400, 50);
}

void ReplayTimelineWidget::replayTimerTimeout()
{
    currentTime += 200;
    while ((currentEvent < replayTimeline.size()) && (replayTimeline[currentEvent] < currentTime)) {
        emit processNextEvent();
        ++currentEvent;
    }
    if (currentEvent == replayTimeline.size()) {
        emit replayFinished();
        replayTimer->stop();
    }
    
    if (!(currentTime % 1000))
        update();
}

void ReplayTimelineWidget::setTimeScaleFactor(qreal _timeScaleFactor)
{
    timeScaleFactor = _timeScaleFactor;
    replayTimer->setInterval(200 / timeScaleFactor);
}

void ReplayTimelineWidget::startReplay()
{
    replayTimer->start(200 / timeScaleFactor);
}

void ReplayTimelineWidget::stopReplay()
{
    replayTimer->stop();
}
