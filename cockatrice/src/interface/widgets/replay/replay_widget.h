/**
 * @file replay_widget.h
 * @ingroup Core
 * @ingroup Replay
 */
//! \todo Document this file.

#ifndef REPLAY_WIDGET_H
#define REPLAY_WIDGET_H

#include "replay_timeline_widget.h"

#include <QToolButton>
#include <QWidget>
#include <libcockatrice/protocol/pb/game_replay.pb.h>

class ReplayQuickSettingsWidget;
class TabGame;

/**
 * @brief The top-level that is put in the replay dock widget.
 * Contains the replay timeline as well as the buttons.
 */
class ReplayWidget : public QWidget
{
    Q_OBJECT

public:
    ReplayWidget(TabGame *parent, GameReplay *replay);
    TabGame *game;
    GameReplay *replay;

signals:
    void requestChatAndPhaseReset();
    void eventReplayed(const GameEventContainer &cont, EventProcessingOptions options);

private:
    // Replay related members
    int currentReplayStep = 0;
    QList<int> replayTimeline;
    ReplayTimelineWidget *timelineWidget;
    QToolButton *replayPlayButton, *replayFastForwardButton;
    ReplayQuickSettingsWidget *settingsWidget;
    QAction *aReplaySkipForward, *aReplaySkipBackward, *aReplaySkipForwardBig, *aReplaySkipBackwardBig;

private slots:
    void replayNextEvent(EventProcessingOptions options);
    void replayFinished();
    void replayPlayButtonToggled(bool checked);
    void updateTimeScaleFactor(bool checked);
    void replayRewind();
    void refreshShortcuts();
};

#endif // REPLAY_WIDGET_H
