#include "replay_manager.h"

#include "../../../client/settings/cache_settings.h"
#include "libcockatrice/protocol/pb/game_event.pb.h"
#include "libcockatrice/protocol/pb/game_event_container.pb.h"
#include "libcockatrice/protocol/pb/game_replay.pb.h"

#include <QTimer>
#include <algorithm>
#include <cmath>
#include <libcockatrice/game/player/event_processing_options.h>
#include <libcockatrice/protocol/get_pb_extension.h>
#include <libcockatrice/settings/interface_settings.h>
#include <limits>
#include <qnumeric.h>

static constexpr int TIMER_INTERVAL_MS = 200;
static constexpr int EMPTY_SECTION_MARGIN_MS = 500;

static QList<int> createReplayTimeline(const GameReplay *replay)
{
    // Create list: event number -> time [ms]
    const int eventCount = replay->event_list_size();

    QList<int> replayTimeline;
    replayTimeline.reserve(eventCount);
    for (int i = 0; i < eventCount; ++i) {
        const auto eventTimestamp = replay->event_list(i).seconds_elapsed();
        int nextSecondIndex = i + 1;
        while (nextSecondIndex < eventCount &&
               replay->event_list(nextSecondIndex).seconds_elapsed() == eventTimestamp) {
            ++nextSecondIndex;
        }

        // Distribute simultaneous events evenly across 1 second.
        const int numberEventsThisSecond = nextSecondIndex - i;
        for (int k = 0; k < numberEventsThisSecond; ++k) {
            const qint64 eventMs = static_cast<qint64>(eventTimestamp) * 1000;
            const int distributionMs = static_cast<int>(static_cast<qint64>(k) * 1000 / numberEventsThisSecond);
            const int time =
                static_cast<int>(std::min<qint64>(eventMs + distributionMs, std::numeric_limits<int>::max()));
            // Preserve recording order even if the replay contains decreasing timestamps.
            replayTimeline.append(std::max(replayTimeline.isEmpty() ? 0 : replayTimeline.last(), time));
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

void ReplayManager::skipToTime(int newTime, bool doRewindBuffering, int targetEventCount)
{
    // check boundary conditions
    if (newTime < 0) {
        newTime = 0;
    }
    if (newTime > maxTime) {
        newTime = maxTime;
    }

    if (targetEventCount < 0) {
        // Preserve coarse timeline navigation, but make the endpoint reachable even
        // when the final event does not fall on a timer interval.
        if (newTime == maxTime) {
            targetEventCount = replayTimeline.size();
        } else {
            newTime -= newTime % TIMER_INTERVAL_MS;
            targetEventCount = static_cast<int>(
                std::lower_bound(replayTimeline.cbegin(), replayTimeline.cend(), newTime) - replayTimeline.cbegin());
        }
    }

    // Event identity matters when several events have the same millisecond timestamp.
    const bool isBackwardsSkip = targetEventCount < currentEvent;
    currentVisualTime = newTime;

    if (isBackwardsSkip) {
        pendingRewindEventCount = targetEventCount;
        handleBackwardsSkip(doRewindBuffering);
    } else {
        // A new forward seek supersedes any buffered backwards request.
        rewindBufferingTimer->stop();
        processNewEvents(FORWARD_SKIP, targetEventCount);
    }

    emit timeChanged(currentVisualTime);
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
        rewindBufferingTimer->start(SettingsCache::instance().userInterface().getRewindBufferingMs());
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
    processNewEvents(BACKWARD_SKIP, pendingRewindEventCount);
}

void ReplayManager::replayTimerTimeout()
{
    // Keep the board and requested position stable until the buffered rewind is applied.
    if (rewindBufferingTimer->isActive()) {
        return;
    }

    currentVisualTime += std::min(TIMER_INTERVAL_MS, maxTime - currentVisualTime);

    processNewEvents(NORMAL_PLAYBACK);

    emit timeChanged(currentVisualTime);

    if (skipEmptySections) {
        handleSkipEmptySection();
    }
}

/** @brief Processes all unprocessed events up to the current time. */
void ReplayManager::processNewEvents(PlaybackMode playbackMode, int targetEventCount)
{
    currentProcessedTime = currentVisualTime;

    if (targetEventCount < 0) {
        targetEventCount = currentProcessedTime == maxTime
                               ? replayTimeline.size()
                               : static_cast<int>(std::lower_bound(replayTimeline.cbegin(), replayTimeline.cend(),
                                                                   currentProcessedTime) -
                                                  replayTimeline.cbegin());
    }

    while (currentEvent < targetEventCount) {
        EventProcessingOptions options;

        // backwards skip => always skip reveal windows
        // forwards skip => skip reveal windows that don't happen within a big skip of the target
        if (playbackMode == BACKWARD_SKIP || currentProcessedTime - replayTimeline[currentEvent] > BIG_SKIP_MS) {
            options |= SKIP_REVEAL_WINDOW;
        }

        // backwards skip => always skip tap animation
        // backwards skip => always skip damage animation (battlefield shimmer / life counter flash)
        if (playbackMode == BACKWARD_SKIP) {
            options |= SKIP_TAP_ANIMATION;
            options |= SKIP_DAMAGE_ANIMATION;
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
    int prevEvent = std::max(0, currentEvent - 1);
    for (; prevEvent > 0 && !hasMeaningfulEvent(replay->event_list(prevEvent)); --prevEvent) {
    }

    int prevEventTime = replayTimeline.value(prevEvent);
    if (currentVisualTime - prevEventTime <= EMPTY_SECTION_MARGIN_MS) {
        return;
    }

    // find next earliest meaningful event
    int nextEvent = currentEvent;
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
    if (!std::isfinite(_timeScaleFactor) || _timeScaleFactor <= 0) {
        return;
    }
    timeScaleFactor = _timeScaleFactor;
    const qreal intervalMs = std::clamp<qreal>(TIMER_INTERVAL_MS / timeScaleFactor, 1, std::numeric_limits<int>::max());
    const int interval = qRound(intervalMs);
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

void ReplayManager::setTimeToEvent(int eventIndex)
{
    if (eventIndex < 0 || eventIndex >= replayTimeline.size()) {
        return;
    }

    // Supersede any buffered rewind and stop immediately after the selected event,
    // including when adjacent events share its timestamp.
    skipToTime(replayTimeline.at(eventIndex), false, eventIndex + 1);
}

void ReplayManager::skipByAmount(int amount)
{
    const qint64 target = static_cast<qint64>(currentVisualTime) + amount;
    skipToTime(static_cast<int>(std::clamp<qint64>(target, 0, maxTime)), amount < 0);
}
