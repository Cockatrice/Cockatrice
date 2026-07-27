/**
 * @file replay_timeline_widget.h
 * @ingroup Replay
 */
//! \todo Document this file.

#ifndef REPLAY_TIMELINE_WIDGET
#define REPLAY_TIMELINE_WIDGET

#include "../../../game/player/event_processing_options.h"

#include <QMouseEvent>
#include <QWidget>

class QPaintEvent;
class QTimer;

class ReplayTimelineWidget : public QWidget
{
    Q_OBJECT

signals:
    void timeClicked(int newTime);

private:
    QList<int> histogram;
    int maxBinValue = 1;
    int maxTime = 0;

    int currentTime = 0;

public:
    explicit ReplayTimelineWidget(QWidget *parent = nullptr);
    void setTimeline(const QList<int> &replayTimeline);
    [[nodiscard]] QSize sizeHint() const override;
    [[nodiscard]] QSize minimumSizeHint() const override;

public slots:
    void setCurrentTime(int time);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
};

#endif
