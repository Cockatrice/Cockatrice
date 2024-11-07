#include "replay_timeline_widget.h"

#include <QPainter>
#include <QPainterPath>
#include <QPalette>
#include <QTimer>

ReplayTimelineWidget::ReplayTimelineWidget(QWidget *parent)
    : QWidget(parent), maxBinValue(1), maxTime(1), timeScaleFactor(1.0), currentTime(0), currentEvent(0)
{
    replayTimer = new QTimer(this);
    connect(replayTimer, SIGNAL(timeout()), this, SLOT(replayTimerTimeout()));

    rewindBufferingTimer = new QTimer(this);
    rewindBufferingTimer->setSingleShot(true);
    connect(rewindBufferingTimer, &QTimer::timeout, this, &ReplayTimelineWidget::processRewind);
}

const int ReplayTimelineWidget::binLength = 5000;

void ReplayTimelineWidget::setTimeline(const QList<int> &_replayTimeline)
{
    replayTimeline = _replayTimeline;
    histogram.clear();
    int binEndTime = binLength - 1;
    int binValue = 0;
    for (int i : replayTimeline) {
        if (i > binEndTime) {
            histogram.append(binValue);
            if (binValue > maxBinValue)
                maxBinValue = binValue;
            while (i > binEndTime + binLength) {
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

    qreal binWidth = (qreal)width() / histogram.size();
    QPainterPath path;
    path.moveTo(0, height() - 1);
    for (int i = 0; i < histogram.size(); ++i)
        path.lineTo(qRound(i * binWidth), (height() - 1) * (1.0 - (qreal)histogram[i] / maxBinValue));
    path.lineTo(width() - 1, height() - 1);
    path.lineTo(0, height() - 1);
    painter.fillPath(path, Qt::black);

    const QColor barColor = QColor::fromHsv(120, 255, 255, 100);
    quint64 w = (quint64)(width() - 1) * (quint64)currentTime / maxTime;
    painter.fillRect(0, 0, static_cast<int>(w), height() - 1, barColor);
}

void ReplayTimelineWidget::mousePressEvent(QMouseEvent *event)
{
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    int newTime = static_cast<int>((qint64)maxTime * (qint64)event->position().x() / width());
#else
    int newTime = static_cast<int>((qint64)maxTime * (qint64)event->x() / width());
#endif
    // don't buffer rewinds from clicks, since clicks usually don't happen fast enough to require buffering
    skipToTime(newTime, false);
}

void ReplayTimelineWidget::skipToTime(int newTime, bool doRewindBuffering)
{
    // check boundary conditions
    if (newTime < 0) {
        newTime = 0;
    }
    if (newTime > maxTime) {
        newTime = maxTime;
    }

    newTime -= newTime % 200; // Time should always be a multiple of 200

    const bool isBackwardsSkip = newTime < currentTime;
    currentTime = newTime;

    if (isBackwardsSkip) {
        handleBackwardsSkip(doRewindBuffering);
    } else {
        processNewEvents();
    }

    update();
}

/// @param doRewindBuffering When true, if multiple backward skips are made in quick succession, only a single rewind
/// is processed at the end. When false, the backwards skip will always cause an immediate rewind
void ReplayTimelineWidget::handleBackwardsSkip(bool doRewindBuffering)
{
    if (doRewindBuffering) {
        // We use a one-shot timer to implement the rewind buffering.
        // The rewind only happens once the timer runs out.
        // If another backwards skip happens, the timer will just get reset instead of rewinding.
        rewindBufferingTimer->stop();
        rewindBufferingTimer->start(calcRewindBufferingTimeout());
    } else {
        // otherwise, process the rewind immediately
        processRewind();
    }
}

/// The timeout scales based on the current event number, up to a limit
int ReplayTimelineWidget::calcRewindBufferingTimeout() const
{
    int extraTime = currentEvent / 100;
    return std::min(BASE_REWIND_BUFFERING_TIMEOUT_MS + extraTime, MAX_REWIND_BUFFERING_TIMEOUT_MS);
}

void ReplayTimelineWidget::processRewind()
{
    // stop any queued-up rewinds
    rewindBufferingTimer->stop();

    // process the rewind
    currentEvent = 0;
    emit rewound();
    processNewEvents();
}

QSize ReplayTimelineWidget::sizeHint() const
{
    return {-1, 50};
}

QSize ReplayTimelineWidget::minimumSizeHint() const
{
    return {400, 50};
}

void ReplayTimelineWidget::replayTimerTimeout()
{
    currentTime += 200;

    processNewEvents();

    if (!(currentTime % 1000))
        update();
}

/// Processes all unprocessed events up to the current time.
void ReplayTimelineWidget::processNewEvents()
{
    while ((currentEvent < replayTimeline.size()) && (replayTimeline[currentEvent] < currentTime)) {
        emit processNextEvent();
        ++currentEvent;
    }
    if (currentEvent == replayTimeline.size()) {
        emit replayFinished();
        replayTimer->stop();
    }
}

void ReplayTimelineWidget::setTimeScaleFactor(qreal _timeScaleFactor)
{
    timeScaleFactor = _timeScaleFactor;
    replayTimer->setInterval(static_cast<int>(200 / timeScaleFactor));
}

void ReplayTimelineWidget::startReplay()
{
    replayTimer->start(static_cast<int>(200 / timeScaleFactor));
}

void ReplayTimelineWidget::stopReplay()
{
    replayTimer->stop();
}

void ReplayTimelineWidget::skipByAmount(int amount)
{
    skipToTime(currentTime + amount, true);
}