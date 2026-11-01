#ifndef COCKATRICE_REPLAY_EVENT_INDEX_H
#define COCKATRICE_REPLAY_EVENT_INDEX_H

#include <QList>
#include <QString>

class GameReplay;

enum class ReplayEventKind
{
    Action,
    Turn,
    Phase,
    Chat,
    System
};

/**
 * @brief One user-visible entry in the replay history sidebar.
 *
 * The event index refers to the corresponding GameEventContainer in the replay. It is used to seek the replay to that
 * container.
 */
struct ReplayEventEntry
{
    int eventIndex = -1;
    int timeMs = 0;
    QString timeLabel;
    QString actor;
    QString badge;
    QString detail;
    ReplayEventKind kind = ReplayEventKind::Action;
};

/**
 * @brief Builds a compact, human-readable index of the events in a replay.
 *
 * The returned entries copy all the data they need, so the caller does not need to keep the replay object alive after
 * this function returns.
 */
QList<ReplayEventEntry> buildReplayEventEntries(const GameReplay *replay, const QList<int> &timeline);

#endif // COCKATRICE_REPLAY_EVENT_INDEX_H
