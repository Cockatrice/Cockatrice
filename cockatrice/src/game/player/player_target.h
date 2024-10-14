#ifndef PLAYERTARGET_H
#define PLAYERTARGET_H

#include "../board/abstract_counter.h"
#include "../board/arrow_target.h"

#include <QFont>
#include <QPixmap>

class Player;

class PlayerCounter : public AbstractCounter
{
    Q_OBJECT
public:
    PlayerCounter(Player *_player,
                  int _id,
                  const QString &_name,
                  int _value,
                  QGraphicsItem *parent = nullptr,
                  QWidget *game = nullptr);
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
};

class PlayerTarget : public ArrowTarget
{
    Q_OBJECT
private:
    QPixmap fullPixmap;
    PlayerCounter *playerCounter;
    QWidget *game;
public slots:
    void counterDeleted();

public:
    enum
    {
        Type = typePlayerTarget
    };
    int type() const
    {
        return Type;
    }

    PlayerTarget(Player *_player = nullptr, QGraphicsItem *parentItem = nullptr, QWidget *_game = nullptr);
    ~PlayerTarget();
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    AbstractCounter *addCounter(int _counterId, const QString &_name, int _value);
};

#endif
