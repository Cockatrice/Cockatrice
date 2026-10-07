/**
 * @file game.h
 * @ingroup GameLogic
 */
//! \todo Document this file.

#ifndef COCKATRICE_GAME_H
#define COCKATRICE_GAME_H

#include "abstract_game.h"

#include <qtmetamacros.h>

class AbstractClient;
class Event_GameJoined;
class QObject;
class QString;
template <class Key, class T> class QMap;
template <typename T> class QList;

class Game : public AbstractGame
{
    Q_OBJECT

public:
    Game(QObject *parent,
         bool isLocalGame,
         QList<AbstractClient *> &_clients,
         const Event_GameJoined &event,
         const QMap<int, QString> &_roomGameTypes);
};

#endif // COCKATRICE_GAME_H
