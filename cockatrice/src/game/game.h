/**
 * @file game.h
 * @ingroup GameLogic
 * @brief TODO: Document this.
 */

#ifndef COCKATRICE_GAME_H
#define COCKATRICE_GAME_H

#include "abstract_game.h"

#include <QObject>

class Game : public AbstractGame
{
    Q_OBJECT

public:
    Game(TabGame *tab,
         QList<AbstractClient *> &_clients,
         const Event_GameJoined &event,
         const QMap<int, QString> &_roomGameTypes);
};

#endif // COCKATRICE_GAME_H
