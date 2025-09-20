#ifndef REPLAY_MANAGER_H
#define REPLAY_MANAGER_H

#include "network/replay_timeline_widget.h"
#include "pb/game_replay.pb.h"

#include <QToolButton>
#include <QWidget>

class TabGame;

class ReplayManager : public QWidget
{
    Q_OBJECT

public:
    ReplayManager(TabGame *parent, GameReplay *replay);
    TabGame *game;
    GameReplay *replay;

signals:
    void requestChatAndPhaseReset();

private:
    // Replay related members
    int currentReplayStep = 0;
    QList<int> replayTimeline;
    ReplayTimelineWidget *timelineWidget;
    QToolButton *replayPlayButton, *replayFastForwardButton;
    QAction *aReplaySkipForward, *aReplaySkipBackward, *aReplaySkipForwardBig, *aReplaySkipBackwardBig;

private slots:
    void replayNextEvent(EventProcessingOptions options);
    void replayFinished();
    void replayPlayButtonToggled(bool checked);
    void replayFastForwardButtonToggled(bool checked);
    void replayRewind();
    void refreshShortcuts();
};

#endif // REPLAY_MANAGER_H
