#ifndef REPLAY_TIMELINE_WIDGET
#define REPLAY_TIMELINE_WIDGET

#include <QWidget>
#include <QList>

class QPaintEvent;
class QTimer;

class ReplayTimelineWidget : public QWidget {
    Q_OBJECT
signals:
    void processNextEvent();
    void replayFinished();
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
    ReplayTimelineWidget(QWidget *parent = 0);
    void setTimeline(const QList<int> &_replayTimeline);
    QSize sizeHint() const;
    QSize minimumSizeHint() const;
    void setTimeScaleFactor(qreal _timeScaleFactor);
    int getCurrentEvent() const { return currentEvent; }
public slots:
    void startReplay();
    void stopReplay();
protected:
    void paintEvent(QPaintEvent *event);
};

#endif
