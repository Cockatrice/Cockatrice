/**
 * @file replay.h
 * @ingroup GameLogic
 * @ingroup Replay
 */
//! \todo Document this file.

#ifndef COCKATRICE_REPLAY_H
#define COCKATRICE_REPLAY_H

#include "abstract_game.h"

class Replay : public AbstractGame
{
    Q_OBJECT

public:
    explicit Replay(QObject *_parent, GameReplay *_replay, bool isLocalGame);
};

#endif // COCKATRICE_REPLAY_H
