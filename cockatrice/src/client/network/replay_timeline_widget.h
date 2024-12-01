#ifndef REPLAY_TIMELINE_WIDGET
#define REPLAY_TIMELINE_WIDGET

#include "../../game/player/player.h"

#include <QList>
#include <QMouseEvent>
#include <QWidget>

class QPaintEvent;
class QTimer;

class ReplayTimelineWidget : public QWidget
{
    Q_OBJECT
signals:
    void processNextEvent(Player::EventProcessingOptions options);
    void replayFinished();
    void rewound();

private:
    enum PlaybackMode
    {
        NORMAL_PLAYBACK,
        FORWARD_SKIP,
        BACKWARD_SKIP
    };

    static constexpr int TIMER_INTERVAL_MS = 200;
    static constexpr int BIN_LENGTH = 5000;
    static constexpr int BASE_REWIND_BUFFERING_TIMEOUT_MS = 180;
    static constexpr int MAX_REWIND_BUFFERING_TIMEOUT_MS = 280;

    QTimer *replayTimer;
    QTimer *rewindBufferingTimer;
    QList<int> replayTimeline;
    QList<int> histogram;
    int maxBinValue, maxTime;
    qreal timeScaleFactor;
    int currentTime;
    int currentEvent;

    void skipToTime(int newTime, bool doRewindBuffering);
    void handleBackwardsSkip(bool doRewindBuffering);
    int calcRewindBufferingTimeout() const;
    void processRewind();
    void processNewEvents(PlaybackMode playbackMode);
private slots:
    void replayTimerTimeout();

public:
    static constexpr int SMALL_SKIP_MS = 1000;
    static constexpr int BIG_SKIP_MS = 10000;
    static constexpr qreal FAST_FORWARD_SCALE_FACTOR = 10.0;

    explicit ReplayTimelineWidget(QWidget *parent = nullptr);
    void setTimeline(const QList<int> &_replayTimeline);
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;
    void setTimeScaleFactor(qreal _timeScaleFactor);
    int getCurrentEvent() const
    {
        return currentEvent;
    }
public slots:
    void startReplay();
    void stopReplay();
    void skipByAmount(int amount); // use a negative amount to skip backwards

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
};

#endif
