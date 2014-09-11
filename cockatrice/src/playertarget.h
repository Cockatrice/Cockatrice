#ifndef PLAYERTARGET_H
#define PLAYERTARGET_H

#include "arrowtarget.h"
#include "abstractcounter.h"
#include <QFont>
#include <QPixmap>

class Player;

class PlayerCounter : public AbstractCounter {
    Q_OBJECT
public:
    PlayerCounter(Player *_player, int _id, const QString &_name, int _value, QGraphicsItem *parent = 0);
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
};

class PlayerTarget : public ArrowTarget {
    Q_OBJECT
private:
    QPixmap fullPixmap;
    PlayerCounter *playerCounter;
public slots:
    void counterDeleted();
public:
    enum { Type = typePlayerTarget };
    int type() const { return Type; }
    
    PlayerTarget(Player *_player = 0, QGraphicsItem *parentItem = 0);
    ~PlayerTarget();
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    
    AbstractCounter *addCounter(int _counterId, const QString &_name, int _value);
};

#endif
