#ifndef REPLAY_TIMELINE_WIDGET
#define REPLAY_TIMELINE_WIDGET

#include <QList>
#include <QMouseEvent>
#include <QWidget>

class QPaintEvent;
class QTimer;

class ReplayTimelineWidget : public QWidget
{
    Q_OBJECT
signals:
    void processNextEvent();
    void replayFinished();
    void rewound();

private:
    QTimer *replayTimer;
    QList<int> replayTimeline;
    QList<int> histogram;
    static const int binLength;
    int maxBinValue, maxTime;
    qreal timeScaleFactor;
    int currentTime;
    int currentEvent;
private slots:
    void replayTimerTimeout();

public:
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

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
};

#endif
