/**
 * @file replay.h
 * @ingroup GameLogic
 * @ingroup Replay
 * @brief TODO: Document this.
 */

#ifndef COCKATRICE_REPLAY_H
#define COCKATRICE_REPLAY_H

#include "abstract_game.h"

class Replay : public AbstractGame
{
    Q_OBJECT

public:
    explicit Replay(TabGame *_tab, GameReplay *_replay);
};

#endif // COCKATRICE_REPLAY_H
