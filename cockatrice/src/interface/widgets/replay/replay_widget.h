/**
 * @file replay_widget.h
 * @ingroup Core
 * @ingroup Replay
 */
//! \todo Document this file.

#ifndef REPLAY_WIDGET_H
#define REPLAY_WIDGET_H

#include "../../../game/player/event_processing_options.h"

#include <QWidget>
#include <qtmetamacros.h>

class ReplayManager;
class ReplayQuickSettingsWidget;
class GameEventContainer;
class GameReplay;
class QAction;
class QToolButton;
class ReplayTimelineWidget;

/**
 * @brief The top-level widget that is put in the replay dock widget.
 * Contains the replay timeline as well as the buttons.
 */
class ReplayWidget : public QWidget
{
    Q_OBJECT

public:
    /**
     * @param parent The parent widget
     * @param replay Cannot be null. Takes ownership of the replay.
     */
    ReplayWidget(QWidget *parent, GameReplay *replay);

signals:
    void rewound();
    void eventReplayed(const GameEventContainer &cont, EventProcessingOptions options);

private:
    ReplayManager *replayManager;

    ReplayTimelineWidget *timelineWidget;
    QToolButton *replayPlayButton, *replayFastForwardButton;
    ReplayQuickSettingsWidget *settingsWidget;
    QAction *aReplaySkipForward, *aReplaySkipBackward, *aReplaySkipForwardBig, *aReplaySkipBackwardBig;

private slots:
    void replayFinished();
    void replayPlayButtonToggled(bool checked);
    void updateTimeScaleFactor(bool checked);
    void refreshShortcuts();
};

#endif // REPLAY_WIDGET_H
