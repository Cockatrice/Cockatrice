#ifndef COCKATRICE_PLAYER_LOGGER_H
#define COCKATRICE_PLAYER_LOGGER_H
#include <QObject>

class PlayerLogger : public QObject
{

    Q_OBJECT
public:
    PlayerLogger(QObject *parent, Player *player);
};

#endif // COCKATRICE_PLAYER_LOGGER_H
