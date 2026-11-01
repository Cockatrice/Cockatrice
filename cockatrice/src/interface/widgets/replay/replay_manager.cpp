#include "replay_manager.h"

#include "../../../client/settings/cache_settings.h"
#include "libcockatrice/protocol/pb/game_event.pb.h"
#include "libcockatrice/protocol/pb/game_event_container.pb.h"
#include "libcockatrice/protocol/pb/game_replay.pb.h"

#include <QTimer>
#include <algorithm>
#include <libcockatrice/game/player/event_processing_options.h>
#include <libcockatrice/protocol/get_pb_extension.h>
#include <libcockatrice/settings/interface_settings.h>
#include <qnumeric.h>

static constexpr int TIMER_INTERVAL_MS = 200;
static constexpr int EMPTY_SECTION_MARGIN_MS = 500;

static QList<int> createReplayTimeline(const GameReplay *replay)
{
    // Create list: event number -> time [ms]
    const int eventCount = replay->event_list_size();

    QList<int> replayTimeline;
    for (int i = 0; i < eventCount; ++i) {
        const auto currentSecond = replay->event_list(i).seconds_elapsed();
        int nextSecondIndex = i + 1;
        while (nextSecondIndex < eventCount && replay->event_list(nextSecondIndex).seconds_elapsed() == currentSecond) {
            ++nextSecondIndex;
        }

        // Distribute simultaneous events evenly across 1 second.
        const int numberEventsThisSecond = nextSecondIndex - i;
        for (int k = 0; k < numberEventsThisSecond; ++k) {
            int eventMs = replay->event_list(i + k).seconds_elapsed() * 1000;
            int distributionMs = static_cast<int>(static_cast<qreal>(k) / numberEventsThisSecond * 1000);
            replayTimeline.append(eventMs + distributionMs);
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
    connect(rewindBufferingTimer, &QTimer::timeout, this, [this] { processRewind(eventsBefore(currentVisualTime)); });
}

ReplayManager::~ReplayManager()
{
    delete replay;
}

int ReplayManager::eventsBefore(int time) const
{
    return static_cast<int>(std::lower_bound(replayTimeline.cbegin(), replayTimeline.cend(), time) -
                            replayTimeline.cbegin());
}

void ReplayManager::skipToTime(int newTime, bool doRewindBuffering)
{
    newTime = std::clamp(newTime, 0, maxTime);
    newTime -= newTime % TIMER_INTERVAL_MS;
    currentVisualTime = newTime;
    const int targetEvent = eventsBefore(newTime);

    if (targetEvent < currentEvent) {
        if (doRewindBuffering) {
            rewindBufferingTimer->start(SettingsCache::instance().userInterface().getRewindBufferingMs());
        } else {
            processRewind(targetEvent);
        }
    } else {
        // The board already represents a position at or before this target. A
        // pending rewind belongs to an earlier selection and must not undo it.
        rewindBufferingTimer->stop();
        processNewEvents(FORWARD_SKIP, targetEvent);
    }
    emit timeChanged(currentVisualTime);
}

void ReplayManager::processRewind(int targetEvent)
{
    rewindBufferingTimer->stop();
    currentEvent = 0;
    emit rewound();
    processNewEvents(BACKWARD_SKIP, targetEvent);
}

void ReplayManager::replayTimerTimeout()
{
    // A buffered rewind leaves the board at the old position. Apply it before
    // playback advances, otherwise the clock and board can diverge.
    if (rewindBufferingTimer->isActive()) {
        processRewind(eventsBefore(currentVisualTime));
    }
    currentVisualTime += TIMER_INTERVAL_MS;
    processNewEvents(NORMAL_PLAYBACK, eventsBefore(currentVisualTime));
    emit timeChanged(currentVisualTime);

    if (skipEmptySections) {
        handleSkipEmptySection();
    }
}

/** @brief Processes events up to the exclusive event index targetEvent. */
void ReplayManager::processNewEvents(PlaybackMode playbackMode, int targetEvent)
{
    while (currentEvent < targetEvent) {
        EventProcessingOptions options;
        if (playbackMode == BACKWARD_SKIP || currentVisualTime - replayTimeline[currentEvent] > BIG_SKIP_MS) {
            options |= SKIP_REVEAL_WINDOW;
        }
        if (playbackMode == BACKWARD_SKIP) {
            options |= SKIP_TAP_ANIMATION;
            options |= SKIP_DAMAGE_ANIMATION;
        }
        emit eventReplayed(replay->event_list(currentEvent), options);
        ++currentEvent;
    }
    emit processedEventChanged(currentEvent - 1);
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

void ReplayManager::setTimeToEvent(int eventIndex)
{
    if (eventIndex < 0 || eventIndex >= replayTimeline.size()) {
        return;
    }

    // Event identity matters when multiple events have the same timestamp.
    // The selected event is included; later events at the same time are not.
    rewindBufferingTimer->stop();
    currentVisualTime = replayTimeline.at(eventIndex);
    const int targetEvent = eventIndex + 1;
    if (targetEvent < currentEvent) {
        processRewind(targetEvent);
    } else {
        processNewEvents(FORWARD_SKIP, targetEvent);
    }
    emit timeChanged(currentVisualTime);
}

void ReplayManager::skipByAmount(int amount)
{
    skipToTime(currentVisualTime + amount, amount < 0);
}
