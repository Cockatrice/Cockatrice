#include "replay_manager.h"

#include "../../../client/settings/cache_settings.h"

#include <QTimer>
#include <libcockatrice/protocol/get_pb_extension.h>
#include <libcockatrice/settings/interface_settings.h>

static constexpr int TIMER_INTERVAL_MS = 200;
static constexpr int EMPTY_SECTION_MARGIN_MS = 500;

static QList<int> createReplayTimeline(const GameReplay *replay)
{
    // Create list: event number -> time [ms]
    unsigned int lastEventTimestamp = 0;
    const int eventCount = replay->event_list_size();

    QList<int> replayTimeline;
    for (int i = 0; i < eventCount; ++i) {
        int nextSecondIndex = i + 1;
        while (nextSecondIndex < eventCount &&
               replay->event_list(nextSecondIndex).seconds_elapsed() == lastEventTimestamp) {
            ++nextSecondIndex;
        }

        // Distribute simultaneous events evenly across 1 second.
        const int numberEventsThisSecond = nextSecondIndex - i;
        for (int k = 0; k < numberEventsThisSecond; ++k) {
            int eventMs = replay->event_list(i + k).seconds_elapsed() * 1000;
            int distributionMs = static_cast<int>(static_cast<qreal>(k) / numberEventsThisSecond * 1000);
            replayTimeline.append(eventMs + distributionMs);
        }

        if (nextSecondIndex < eventCount) {
            lastEventTimestamp = replay->event_list(nextSecondIndex).seconds_elapsed();
        }
        i += numberEventsThisSecond - 1;
    }

    return replayTimeline;
}

ReplayManager::ReplayManager(QObject *parent, GameReplay *replay)
    : QObject(parent), replay(replay), replayTimeline(createReplayTimeline(replay))
{
    maxTime = replayTimeline.isEmpty() ? 0 : replayTimeline.last();

    replayTimer = new QTimer(this);
    replayTimer->setInterval(TIMER_INTERVAL_MS);
    connect(replayTimer, &QTimer::timeout, this, &ReplayManager::replayTimerTimeout);

    rewindBufferingTimer = new QTimer(this);
    rewindBufferingTimer->setSingleShot(true);
    connect(rewindBufferingTimer, &QTimer::timeout, this, &ReplayManager::processRewind);
}

ReplayManager::~ReplayManager()
{
    delete replay;
}

void ReplayManager::skipToTime(int newTime, bool doRewindBuffering)
{
    // check boundary conditions
    if (newTime < 0) {
        newTime = 0;
    }
    if (newTime > maxTime) {
        newTime = maxTime;
    }

    newTime -= newTime % TIMER_INTERVAL_MS; // Time should always be a multiple of the interval

    const bool isBackwardsSkip = newTime < currentProcessedTime;
    currentVisualTime = newTime;

    if (isBackwardsSkip) {
        handleBackwardsSkip(doRewindBuffering);
    } else {
        processNewEvents(FORWARD_SKIP);
    }

    timeChanged(currentVisualTime);
}

/**
 * @brief Handles a backwards skip in the replay timeline.
 *
 * @param doRewindBuffering When true, if multiple backward skips are made in quick succession, only a single rewind
 * is processed at the end. When false, the backwards skip will always cause an immediate rewind.
 */
void ReplayManager::handleBackwardsSkip(bool doRewindBuffering)
{
    if (doRewindBuffering) {
        // We use a one-shot timer to implement the rewind buffering.
        // The rewind only happens once the timer runs out.
        // If another backwards skip happens, the timer will just get reset instead of rewinding.
        rewindBufferingTimer->stop();
        rewindBufferingTimer->start(SettingsCache::instance().interface().getRewindBufferingMs());
    } else {
        // otherwise, process the rewind immediately
        processRewind();
    }
}

void ReplayManager::processRewind()
{
    // stop any queued-up rewinds
    rewindBufferingTimer->stop();

    // process the rewind
    currentEvent = 0;
    emit rewound();
    processNewEvents(BACKWARD_SKIP);
}

void ReplayManager::replayTimerTimeout()
{
    currentVisualTime += TIMER_INTERVAL_MS;

    processNewEvents(NORMAL_PLAYBACK);

    timeChanged(currentVisualTime);

    if (skipEmptySections) {
        handleSkipEmptySection();
    }
}

/** @brief Processes all unprocessed events up to the current time. */
void ReplayManager::processNewEvents(PlaybackMode playbackMode)
{
    currentProcessedTime = currentVisualTime;

    while (currentEvent < replayTimeline.size() && replayTimeline[currentEvent] < currentProcessedTime) {
        EventProcessingOptions options;

        // backwards skip => always skip reveal windows
        // forwards skip => skip reveal windows that don't happen within a big skip of the target
        if (playbackMode == BACKWARD_SKIP || currentProcessedTime - replayTimeline[currentEvent] > BIG_SKIP_MS) {
            options |= SKIP_REVEAL_WINDOW;
        }

        // backwards skip => always skip tap animation
        if (playbackMode == BACKWARD_SKIP) {
            options |= SKIP_TAP_ANIMATION;
        }

        emit eventReplayed(replay->event_list(currentEvent), options);
        ++currentEvent;
    }
    if (currentEvent == replayTimeline.size()) {
        emit replayFinished();
        replayTimer->stop();
    }
}

static bool hasMeaningfulEvent(const GameEventContainer &cont)
{
    const int eventListSize = cont.event_list_size();
    for (int i = 0; i < eventListSize; ++i) {
        const GameEvent &event = cont.event_list(i);
        const auto eventType = static_cast<GameEvent::GameEventType>(getPbExtension(event));

        if (eventType != GameEvent::PLAYER_PROPERTIES_CHANGED) {
            return true;
        }
    }

    return false;
}

void ReplayManager::handleSkipEmptySection()
{
    if (currentEvent == replayTimeline.size()) {
        return;
    }

    // find most recent meaningful event
    int prevEvent = currentEvent;
    for (; prevEvent > 0 && !hasMeaningfulEvent(replay->event_list(prevEvent)); --prevEvent) {
    }

    int prevEventTime = replayTimeline.value(prevEvent);
    if (currentVisualTime - prevEventTime <= EMPTY_SECTION_MARGIN_MS) {
        return;
    }

    // find next earliest meaningful event
    int nextEvent = currentEvent + 1;
    for (; nextEvent < replayTimeline.size() - 1 && !hasMeaningfulEvent(replay->event_list(nextEvent)); ++nextEvent) {
    }

    int nextEventTime = replayTimeline.value(nextEvent);
    if (nextEventTime - currentVisualTime <= EMPTY_SECTION_MARGIN_MS) {
        return;
    }

    // skip forward if we're not within margin of either event
    skipToTime(nextEventTime - EMPTY_SECTION_MARGIN_MS, false);
}

void ReplayManager::setTimeScaleFactor(qreal _timeScaleFactor)
{
    timeScaleFactor = _timeScaleFactor;
    int interval = std::max(1, qRound(TIMER_INTERVAL_MS / timeScaleFactor));
    replayTimer->setInterval(interval);
}

void ReplayManager::setSkipEmptySections(bool value)
{
    skipEmptySections = value;
}

void ReplayManager::startReplay()
{
    replayTimer->start();
}

void ReplayManager::stopReplay()
{
    replayTimer->stop();
}

void ReplayManager::setTime(int time)
{
    // don't buffer rewinds from clicks, since clicks usually don't happen fast enough to require buffering
    skipToTime(time, false);
}

void ReplayManager::skipByAmount(int amount)
{
    skipToTime(currentVisualTime + amount, amount < 0);
}