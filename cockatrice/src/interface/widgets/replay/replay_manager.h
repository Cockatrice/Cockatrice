#ifndef COCKATRICE_REPLAY_MANAGER_H
#define COCKATRICE_REPLAY_MANAGER_H

#include "../../../game/player/event_processing_options.h"

#include <QObject>
#include <libcockatrice/protocol/pb/game_replay.pb.h>

class GameReplay;
class QTimer;

/**
 * @brief This class handles all logic to do with playing back replays
 */
class ReplayManager : public QObject
{
    Q_OBJECT

    enum PlaybackMode
    {
        NORMAL_PLAYBACK,
        FORWARD_SKIP,
        BACKWARD_SKIP
    };

    GameReplay *replay;
    QList<int> replayTimeline; ///< timestamp of each event, with the indexes corresponding
    int maxTime;

    QTimer *replayTimer;
    QTimer *rewindBufferingTimer;

    qreal timeScaleFactor = 1.0;

    int currentVisualTime = 0;    ///< time currently displayed by the timeline
    int currentProcessedTime = 0; ///< time that events are currently processed up to. Could differ from visual time due
                                  ///< to rewind buffering
    int currentEvent = 0;         ///< current event's index

    void skipToTime(int newTime, bool doRewindBuffering);
    void handleBackwardsSkip(bool doRewindBuffering);
    void processRewind();
    void processNewEvents(PlaybackMode playbackMode);

private slots:
    void replayTimerTimeout();

public:
    static constexpr int SMALL_SKIP_MS = 1000;
    static constexpr int BIG_SKIP_MS = 10000;

    /**
     * @param parent The parent QObject
     * @param replay Cannot be null. Takes ownership of the object.
     */
    explicit ReplayManager(QObject *parent, GameReplay *replay);

    ~ReplayManager() override;

    const QList<int> &getReplayTimeline() const
    {
        return replayTimeline;
    }

    void setTimeScaleFactor(qreal _timeScaleFactor);

public slots:
    void startReplay();
    void stopReplay();
    void setTime(int time);
    void skipByAmount(int amount); // use a negative amount to skip backwards

signals:
    void timeChanged(int time);
    void eventReplayed(const GameEventContainer &cont, EventProcessingOptions options);
    void replayFinished();
    void rewound();
};

#endif // COCKATRICE_REPLAY_MANAGER_H
