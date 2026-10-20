#ifndef COCKATRICE_REPLAY_EVENT_SIDEBAR_H
#define COCKATRICE_REPLAY_EVENT_SIDEBAR_H

#include <QList>
#include <QWidget>
#include <qtmetamacros.h>

class GameReplay;
class QLabel;
class QListWidget;

/**
 * @brief A scrollable list of replay events that can be used to seek the replay.
 *
 * The sidebar copies the event data it needs at construction time. It does not keep a reference to the replay or to
 * the replay manager.
 */
class ReplayEventSidebar : public QWidget
{
    Q_OBJECT

public:
    /**
     * @param parent The parent widget
     * @param replay The replay to build the event list from
     * @param timeline Event index to time in milliseconds, as used by ReplayManager
     */
    ReplayEventSidebar(QWidget *parent, const GameReplay *replay, const QList<int> &timeline);

public slots:
    /**
     * @brief Highlights the row of the event the game state has reached.
     *
     * @param eventIndex Index into the replay's event list, or -1 when nothing has been replayed yet
     */
    void setCurrentEvent(int eventIndex);

signals:
    void eventSelected(int eventIndex);

private:
    void setCurrentRow(int row);

    QListWidget *eventList;
    QLabel *emptyLabel;
    QList<int> eventIndices; ///< Replay event index of each row, ascending
    int currentRow = -1;
};

#endif // COCKATRICE_REPLAY_EVENT_SIDEBAR_H
